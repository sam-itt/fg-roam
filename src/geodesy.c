#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "geodesy.h"
#include "geo-location.h"

static double EARTH_A = NAN;
static double EARTH_B = NAN;
static double EARTH_F = NAN;
static double EARTH_Ecc = NAN;
static double EARTH_Esq = NAN;

/**
 * Sets Earth Constants as globals
 *
 * <p>Inits globals EARTH_*</p>
 *
 * @param a
 * @param b
 */
static void earthcon(double a, double b)
{
   double  f,ecc, eccsq;


   f        =  1-b/a;
   eccsq    =  1 - b*b/(a*a);
   ecc      =  sqrt(eccsq);

   EARTH_A  =  a;
   EARTH_B  =  b;
   EARTH_F  =  f;
   EARTH_Ecc=  ecc;
   EARTH_Esq=  eccsq;
}

/**
 * WGS84 Earth Constants
 *
 * <p>Inits global earth constants using WGS84 settings</p>
 *
 */
static void wgs84(void)
{
  double  wgs84a, wgs84b, wgs84f;

  wgs84a         =  6378.137;
  wgs84f         =  1.0/298.257223563;
  wgs84b         =  wgs84a * ( 1.0 - wgs84f );

  earthcon (wgs84a, wgs84b);
}

/**
 * Test and ensure geodesy globals loaded
 *
 */
static void geodGBL(void)
{
    if(isnan(EARTH_A))
        wgs84();
}


/**
 * Compute the radii at the geodetic latitude lat (in degrees)
 *
 * <p>Returns a pointer to 3 doubles r, rn, rm (all in km).
 * The caller must not free the returned value.</p>
 *
 * @param lat geodetic latitude in degrees
 * @return    static array of 3 doubles: r, rn, rm in km.
 */
static const double *radcur(double lat)
{
    static double rrnrm[3] = {NAN,NAN,NAN};

    double dtr   = M_PI/180.0;

    double  a,b;
    double  asq,bsq,eccsq,ecc,clat,slat;
    double  dsq,d,rn,rm,rho,rsq,r,z;

    //        -------------------------------------

    geodGBL();

    a     = EARTH_A;
    b     = EARTH_B;

    asq   = a*a;
    bsq   = b*b;
    eccsq  =  1 - bsq/asq;
    ecc = sqrt(eccsq);

    clat  =  cos(dtr*lat);
    slat  =  sin(dtr*lat);

    dsq   =  1.0 - eccsq * slat * slat;
    d     =  sqrt(dsq);

    rn    =  a/d;
    rm    =  rn * (1.0 - eccsq ) / dsq;

    rho   =  rn * clat;
    z     =  (1.0 - eccsq ) * rn * slat;
    rsq   =  rho*rho + z*z;
    r     =  sqrt( rsq );

    rrnrm[0]  =  r;
    rrnrm[1]  =  rn;
    rrnrm[2]  =  rm;

    return(rrnrm);
}

/**
 * Physical radius of earth from geodetic latitude
 */
static double rearth(double lat)
{
    double r;
    const double *rrnrm;


    rrnrm =  radcur(lat);
    r     =  rrnrm[0];

    return ( r );
}


/**
 * Geocentric latitude to geodetic latitude
 *
 * @param flatgc geocentric latitude in degrees
 * @param altkm  altitude in km
 * @return       geodetic latitude in degrees
 */
static double gc2gd (double flatgc, double altkm)
{
    double dtr   = M_PI/180.0;
    double rtd   = 1/dtr;

    double  flatgd;
    double  re,rn,ecc, esq;
    double  slat,clat,tlat;
    double  altnow,ratio;
    const double *rrnrm;

    geodGBL();

    ecc   =  EARTH_Ecc;
    esq   =  ecc*ecc;

    //             approximation by stages
    //             1st use gc-lat as if is gd, then correct alt dependence
    altnow  =  altkm;

    rrnrm   =  radcur(flatgc);
    rn      =  rrnrm[1];

    ratio   = 1 - esq*rn/(rn+altnow);

    tlat    = tan(dtr*flatgc) / ratio;
    flatgd  = rtd * atan(tlat);

    //        now use this approximation for gd-lat to get rn etc.
    rrnrm   =  radcur(flatgd);
    rn      =  rrnrm[1];

    ratio   =  1  - esq*rn/(rn+altnow);
    tlat    =  tan(dtr*flatgc)/ratio;
    flatgd  =  rtd * atan(tlat);

    return  flatgd;
}




/**
 * xyz vector to lat,lon,height
 *
 * <p>Returns a pointer to 3 doubles (lat,lng, alt).
 * Geodetic Latitude and longitude are in degrees, while altitude is in km.
 * Pointerer references local static memory, the caller must not
 * free it.</p>
 *
 * @param xvec   xyz ECEF location
 * @return       pointer to 3 doubles
 */
const double *xyzllh(double *xvec)
{

    double  dtr =  M_PI/180.0;
    double  flatgc,flatn,dlat;
    double  rnow,rp;
    double  x,y,z,p;
    double  tangc,tangd;

    double  testval,kount;

    double  rn,esq;
    double  clat,slat;
    const double  *rrnrm;

    double  flat,flon,altkm;
    static double  llhvec[3] = {NAN,NAN,NAN};


    geodGBL();

    esq    =  EARTH_Esq;

    x      = xvec[0];
    y      = xvec[1];
    z      = xvec[2];

    rp     = sqrt( x*x + y*y + z*z );

    flatgc = asin( z / rp )/dtr;

    testval= fabs(x) + fabs(y);
    if ( testval < 1.0e-10)
     {flon = 0.0; }
    else
     {flon = atan2( y,x )/dtr; }
    if (flon < 0.0 )  { flon = flon + 360.0; }

    p      =  sqrt( x*x + y*y );

    //             on pole special case

    if ( p < 1.0e-10 )
    {
      flat = 90.0;
      if ( z < 0.0 ) { flat = -90.0; }

      altkm = rp - rearth(flat);
      llhvec[0]  = flat;
      llhvec[1]  = flon;
      llhvec[2]  = altkm;

      return  llhvec;
    }

    //        first iteration, use flatgc to get altitude
    //        and alt needed to convert gc to gd lat.

    rnow  =  rearth(flatgc);
    altkm =  rp - rnow;
    flat  =  gc2gd (flatgc,altkm);

    rrnrm =  radcur(flat);
    rn    =  rrnrm[1];

    for ( int kount = 0; kount< 5 ; kount++ ){
       slat  =  sin(dtr*flat);
       tangd =  ( z + rn*esq*slat ) / p;
       flatn =  atan(tangd)/dtr;

       dlat  =  flatn - flat;
       flat  =  flatn;
       clat  =  cos( dtr*flat );

       rrnrm =  radcur(flat);
       rn    =  rrnrm[1];

       altkm =  (p/clat) - rn;

       if ( fabs(dlat) < 1.0e-12 ) { break; }

    }

    llhvec[0]  = flat;
    llhvec[1]  = flon;
    llhvec[2]  = altkm;

    return  llhvec ;
}



/**
 * lat,lon,height to xyz vector
 *
 * <p>Returns a pointer to 3 doubles (x,y,z) all in km.
 * Pointerer references local static memory, the caller must not
 * free it.</p>
 *
 * @param flat   geodetic latitude in degree
 * @param flon   longitude in degrees
 * @param altkm  altitude in km
 * @return       pointer to 3 doubles
 */
const double *llhxyz(double flat, double flon, double altkm)
{

    double  dtr =  M_PI/180.0;
    double  clat,clon,slat,slon;
    const double  *rrnrm;
    double  rn,esq;
    double  x,y,z;
    double re, ecc;
    static double xvec[3] = {NAN,NAN,NAN};

    geodGBL();

    clat = cos(dtr*flat);
    slat = sin(dtr*flat);
    clon = cos(dtr*flon);
    slon = sin(dtr*flon);

    rrnrm  = radcur(flat);
    rn     = rrnrm[1];
    re     = rrnrm[0];

    ecc    = EARTH_Ecc;
    esq    = ecc*ecc;

    x      =  (rn + altkm) * clat * clon;
    y      =  (rn + altkm) * clat * slon;
    z      =  ( (1-esq)*rn + altkm ) * slat;

    xvec[0]  = x;
    xvec[1]  = y;
    xvec[2]  = z;

    return xvec;
}

/**
 * Computes the shortest distance (km) between two (lat/lon) points,
 * using haversine formula.
 *
 * @param lat1 first point latitude in degrees
 * @param lon1 first point longitude in degrees
 * @param lat2 second point latitude in degrees
 * @param lon2 second point longitude in degrees
 * @return distance between the two in km.
 */
double geo_dist(double lat1, double lon1, double lat2, double lon2)
{
	double dx, dy, dz;
	lon1 -= lon2;
	lon1 *= (M_PI / 180.0);
    lat1 *= (M_PI / 180.0);
    lat2 *= (M_PI / 180.0);

	dz = sin(lat1) - sin(lat2);
	dx = cos(lon1) * cos(lat1) - cos(lat2);
	dy = sin(lon1) * cos(lat1);
	return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * EARTH_RADIUS;
}


/**
 * Computes the bearing/heading from point1 to point2, both (lat/lon) in degrees.
 *
 * @param lat1 first point latitude in degrees
 * @param lon1 first point longitude in degrees
 * @param lat2 second point latitude in degrees
 * @param lon2 second point longitude in degrees
 * @return Bearing/Heading between the two in degrees.
 */
double geo_bearing(double lat1, double lon1, double lat2, double lon2)
{
    double rv;

    /*Convert all coordinates in radians*/
    lat1 *= (M_PI / 180.0);
 	lon1 *= (M_PI / 180.0);
    lat2 *= (M_PI / 180.0);
 	lon2 *= (M_PI / 180.0);

    double dLon = (lon2 - lon1);

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dLon);

    rv = atan2(y, x);
    /*Radians to degrees*/
    rv *= (180.0 / M_PI);
    rv = fmod(rv + 360, 360);
//    rv = (rv + 360) % 360;
//    rv = 360 - rv; // count degrees counter-clockwise - remove to make clockwise
    return rv;
}

/**
 * Computes the landing destination (lat/lon) where starting from
 * olat/olon with bearing and going distance km.
 *
 * <p>Returns a pointer to 3 doubles (x,y,z) all in km.
 * Pointerer references local static memory, the caller must not
 * free it.</p>
 *
 * @param olat origin point latitude in degrees
 * @param olon origin point longitude in degrees
 * @param distance distance walked in km
 * @param bearing orientation at the start point
 * @return       pointer to 2 doubles (lat,lon), not to be freed
 */
double *geo_destination(double olat, double olon, double distance, double bearing)
{
    static double rv[2];

    //Earth Radious in KM
   // double R = 6378.14;
    double R = EARTH_RADIUS;

    //convert to radians
    olat *= (M_PI/180);
    olon *= (M_PI/180);
    bearing *= (M_PI/180);

    rv[0] = asin(sin(olat)*cos(distance/R) + cos(olat)*sin(distance/R)*cos(bearing));
    rv[1] = olon + atan2(sin(bearing)*sin(distance/R)*cos(olat), cos(distance/R)-sin(olat)*sin(rv[0]));

    //convert to degrees
    rv[0] *= (180/M_PI);
    rv[1] *= (180/M_PI);

    return rv;
}


/*Teta in rads*/

/**
 * Main equation from
 * Using Rotations to Build Aerospace Coordinate Systems
 * Don Koks (DSTO-TN-0640)
 *
 * @param n vector to rotate
 * @param teta angle(IN RADIANS) of the rotation
 * @param result resulting rotation matrix to be used to
 *               apply the rotation
 */
void geo_mat3_rot(vec3 n, float teta, mat3 result)
{
    mat3 m2 = GLM_MAT3_IDENTITY_INIT;
    mat3 nX = {
        {0, n[2], -n[1]},
        {-n[2], 0, n[0]},
        {n[1], -n[0], 0}
    };

    /* Classic math representation
     * n x nt :
     * n[0]
     * n[1] X n[0] n[1] n[2]
     * n[2]
     *
     * n[0]*n[0]  n[0]*n[1]  n[0]*n[2]
     * n[1]*n[0]  n[1]*n[1]  n[1]*n[2]
     * n[2]*n[0]  n[2]*n[1]  n[2]*n[2]
    */
    /*Turned col major:*/
    mat3 nnt = {
        {n[0]*n[0], n[1]*n[0], n[2]*n[0]},
        {n[0]*n[1], n[1]*n[1], n[2]*n[1]},
        {n[0]*n[2], n[1]*n[2], n[2]*n[2]}
    };


    glm_mat3_scale(nnt, 1.0f - cosf(teta));
    glm_mat3_scale(m2, cosf(teta));
    glm_mat3_scale(nX, sinf(teta));

    glm_mat3_zero(result);
    glm_vec3_add(nnt[0], m2[0], result[0]);
    glm_vec3_add(nnt[1], m2[1], result[1]);
    glm_vec3_add(nnt[2], m2[2], result[2]);

    glm_vec3_add(result[0], nX[0], result[0]);
    glm_vec3_add(result[1], nX[1], result[1]);
    glm_vec3_add(result[2], nX[2], result[2]);
}


void geo_get_ned(double lat, double lon, vec3 n, vec3 e, vec3 d)
{
    mat3 mtmp;

    vec3 n0 = {0,0,1};
    vec3 e0 = {0,1,0};
    vec3 d0 = {-1,0,0};

    vec3 n1, e1, d1;

    /*Intermediate coords*/
    glm_mat3_zero(mtmp);
    geo_mat3_rot(n0, glm_rad(lon), mtmp);
//    printf("rn0: \n");
//    mat3_dump(mtmp);
    glm_mat3_mulv(mtmp, n0, n1);
//    printf("n1 :");
//    vec3_dump(n1);
    glm_mat3_mulv(mtmp, e0, e1);
//    printf("e1 :");
//    vec3_dump(e1);
    glm_mat3_mulv(mtmp, d0, d1);
//    printf("d1 :");
//    vec3_dump(d1);



    /*final coords*/
    vec3 e1_neg;
    glm_vec3_negate_to(e1, e1_neg);
    glm_mat3_identity(mtmp);
    geo_mat3_rot(e1_neg, glm_rad(lat), mtmp);
    glm_mat3_mulv(mtmp, n1, n);
    glm_mat3_mulv(mtmp, e1, e);
    glm_mat3_mulv(mtmp, d1, d);
}

/**
 * return: X, Y, Z, psi, teta, phi
 * alt in meters
 */
const double *geo_llh_to_dis(double lat, double lon, float alt, float heading, float pitch, float roll)
{
    static double rv[6];
    const double *pos;
    double psi, theta, phi;
    vec3 n,e,d;
    vec3 x0,y0,z0;
    vec3 x1,y1,z1;
    vec3 x2,y2,z2;
    vec3 x3,y3,z3;
    mat3 mtmp;

    pos = llhxyz(lat, lon, alt/1000.0);
    rv[0] = pos[0];
    rv[1] = pos[1];
    rv[2] = pos[2];

    geo_get_ned(lat, lon, n, e, d);
    glm_vec3_copy(n, x0);
    glm_vec3_copy(e, y0);
    glm_vec3_copy(d, z0);

    /*Heading*/
    glm_mat3_zero(mtmp);
    geo_mat3_rot(z0, glm_rad(heading), mtmp);
//    printf("rz0: \n");
//    mat3_dump(mtmp);
    glm_mat3_mulv(mtmp, x0, x1);
//    printf("x1 :");
//    vec3_dump(x1);
    glm_mat3_mulv(mtmp, y0, y1);
//    printf("y1 :");
//    vec3_dump(y1);
    glm_mat3_mulv(mtmp, z0, z1);
//    printf("z1 :");
//    vec3_dump(z1);

    /*pitch*/
    glm_mat3_zero(mtmp);
    geo_mat3_rot(y1, glm_rad(pitch), mtmp);
//    printf("rzy1: \n");
//    mat3_dump(mtmp);
    glm_mat3_mulv(mtmp, x1, x2);
//    printf("x2 :");
//    vec3_dump(x2);
    glm_mat3_mulv(mtmp, y1, y2);
//    printf("y2 :");
//    vec3_dump(y2);
    glm_mat3_mulv(mtmp, z1, z2);
//    printf("z2 :");
//    vec3_dump(z2);

    /*roll*/
    glm_mat3_zero(mtmp);
    geo_mat3_rot(x2, glm_rad(roll), mtmp);
//    printf("rzx2: \n");
//    mat3_dump(mtmp);
    glm_mat3_mulv(mtmp, x2, x3);
    printf("x3 :");
    vec3_dump(x3);
    glm_mat3_mulv(mtmp, y2, y3);
    printf("y3 :");
    vec3_dump(y3);
    glm_mat3_mulv(mtmp, z2, z3);
    printf("z3 :");
    vec3_dump(z3);

    glm_vec3_copy((vec3){1,0,0}, x0);
    glm_vec3_copy((vec3){0,1,0}, y0);
    glm_vec3_copy((vec3){0,0,1}, z0);
/*

    printf("Doing angles with:\n");
    printf("x0 :");
    vec3_dump(x0);
    printf("y0 :");
    vec3_dump(y0);
    printf("z0 :");
    vec3_dump(z0);
    printf("x3 :");
    vec3_dump(x3);
    printf("y3 :");
    vec3_dump(y3);
    printf("z3 :");
    vec3_dump(z3);
-*/


#if 0
    psi = asin(glm_vec3_dot(x3,y0)/sqrt(glm_vec3_dot(x3,x0)*glm_vec3_dot(x3,x0) + glm_vec3_dot(x3,y0)*glm_vec3_dot(x3,y0)));
    theta = asin(-1*glm_vec3_dot(x3,z0));
    phi = asin(glm_vec3_dot(y3,z2));
#else
    psi = atan2(glm_vec3_dot(x3,y0), glm_vec3_dot(x3,x0));
    theta = atan2(-1*glm_vec3_dot(x3,z0), sqrt(glm_vec3_dot(x3,x0)*glm_vec3_dot(x3,x0) + glm_vec3_dot(x3,y0)*glm_vec3_dot(x3,y0)));
    phi = atan2(glm_vec3_dot(y3,z2), glm_vec3_dot(y3,y2));
#endif
    rv[3] = glm_deg(psi);
    rv[4] = glm_deg(theta);
    rv[5] =  glm_deg(phi);
    printf("psi(roll): %0.1f,theta(yaw): %0.1f,phi(pitch): %0.1f\n",(psi),(theta),(phi));
    return rv;
}


void vec3_dump(vec3 v)
{
    for(int i = 0; i < 3; i++)
        printf("% -.2f ", v[i]);
    printf("\n");
}

void vec3_dump_long(vec3 v)
{
    for(int i = 0; i < 3; i++)
        printf("% -f ", v[i]);
    printf("\n");
}


void mat3_dump(mat3 m)
{
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++)
            printf("% -.2f ", m[j][i]);
        printf("\n");
    }
    printf("\n");
}


void mat4_dump(mat4 m)
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++)
            printf("% -.2f ", m[j][i]);
        printf("\n");
    }
    printf("\n");
}

void mat4d_dump(mat4d m)
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++)
            printf("% -.2f ", m[j][i]);
        printf("\n");
    }
    printf("\n");
}


void mat4_dump_full(mat4 m)
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++)
            printf("% -f ", m[j][i]);
        printf("\n");
    }
    printf("\n");
}

void mat4d_dump_full(mat4d m)
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++)
            printf("% -f ", m[j][i]);
        printf("\n");
    }
    printf("\n");
}

bool same_sign(float a, float b)
{
    return a*b >= 0.0f;
}

float local_round(float var)
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

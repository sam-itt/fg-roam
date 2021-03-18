#! /usr/bin/python
import sys

from xml.etree import ElementTree as ET


if len(sys.argv) < 2:
    print("Usage: %s materials.xml" % sys.argv[0])
    sys.exit(-1)

xmlfile = sys.argv[1]
#xmlfile='/usr/share/flightgear/Materials/base/materials-base.xml'
doc = ET.parse(xmlfile).getroot()

for material in doc.findall('material'):
    try:
        tex_node = material.find('./texture')
        if tex_node is None:
            tex_node = material.find('./texture-set/texture')
        if tex_node is None:
            continue
        texture = tex_node.text
        for name in material.findall('./name'):
            print("%s:%s" % (name.text,texture))
    except:
        print(ET.tostring(material,'unicode'))
        raise



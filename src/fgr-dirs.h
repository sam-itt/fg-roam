/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef FGR_DIRS_H
#define FGR_DIRS_H

#ifndef FGR_HOME
#define FGR_HOME "."
#endif

#ifndef SHADER_DIR
#if USE_GLES
#define SHADER_DIR FGR_HOME"/shaders/gles"
#else
#define SHADER_DIR FGR_HOME"/shaders/gl"
#endif
#endif

#ifndef SKY_DIR
#define SKY_DIR FGR_HOME"/resources/skybox"
#endif

#ifndef TERRAIN_DIR
#define TERRAIN_DIR FGR_HOME"/resources/fg-scenery/Terrain"
#endif

#ifndef TEX_DIR
#if USE_TINY_TEXTURES
#define TEX_DIR FGR_HOME"/resources/fg-scenery/textures/small"
#else
#define TEX_DIR FGR_HOME"/resources/fg-scenery/textures/full"
#endif //USE_TINY_TEXTURES
#endif //TEX_DIR

#endif /* FGR_DIRS_H */

#pragma once


float ground[] =
{
    // Position           // Texture coordinates
                          // on veut quelles se répètent 5 fois donc on met les coor des tex à 5.0
    -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
     0.5f, 0.0f, -0.5f,   5.0f, 0.0f,
     0.5f, 0.0f,  0.5f,   5.0f, 5.0f,
    -0.5f, 0.0f,  0.5f,   0.0f, 5.0f
};

unsigned int planeElements[] =
{
    0, 2, 1,
    0, 3, 2
};

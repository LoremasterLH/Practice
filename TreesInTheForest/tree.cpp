#include "tree.h"
#include <math.h>
#include <random>

const float viewDistance = 200.0F; // Assuming distance of view.
const float PI = 3.14159265358979323846F;
const int num_trees = 25;

Tree::Tree(float X, float Y)
{
    x = X;
    y = Y;
}

float RadianToDegrees(const float &radians)
{
    return radians * 180 / PI;
}

float DegreesToRadian(const float &degrees)
{
    return degrees * PI / 180;
}

Tree GetBorderPosition(const float &angle)  // Returns point at the edge of viewDistance.
{
    return Tree(viewDistance * static_cast<float>(cos(static_cast<double>(angle))), viewDistance * static_cast<float>(sin(static_cast<double>(angle))));
}

float IsLeft(const Tree &tree, const Tree &boundary)  // Tree and coordinates of the line to check against.
{
    return boundary.x * tree.y - boundary.y * tree.x;
}

bool IsVisible(const Tree &tree, const float &viewAngle, const float &direction)
{
    if (IsLeft(tree, GetBorderPosition(direction-viewAngle)) >= 0 &&
            IsLeft(tree, GetBorderPosition(direction)) <= 0)
        return true;
    else
        return false;
}

float GetAngle(const Tree &tree, const float &viewAngle)
{
    return static_cast<float>(atan2(static_cast<double>(tree.y), static_cast<double>(tree.x))) - viewAngle / 2.F;
}

int GetQuadrant(float x, float y)
{
    if(x>=0 && y>=0)
        return 1;
    else if(x<=0 && y>=0)
        return 2;
    else if(x<0 && y<0)
        return 3;
    else
        return 4;
}

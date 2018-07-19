#include "dialog.h"
#include "ui_dialog.h"
#include <iostream>
#include <list>
#include <math.h>
#include <random>

using namespace std;

const float viewDistance = 200.0F; // Assuming distance of view.
const float PI = 3.14159265358979323846F;
const int num_trees = 25;

class ATreePosition
{
public:
    float	x,y;
    ATreePosition() {}
    ATreePosition(float X, float Y)
    {
        x = X;
        y = Y;
    }
};

float RadianToDegrees(const float &radians)
{
    return radians * 180 / PI;
}

float DegreesToRadian(const float &degrees)
{
    return degrees * PI / 180;
}

ATreePosition GetBorderPosition(const float &angle)  // Returns point at the edge of viewDistance.
{
    return ATreePosition(viewDistance * cos(angle), viewDistance * sin(angle));
}

float IsLeft(const ATreePosition &tree, const ATreePosition &boundary)  // Tree and coordinates of the line to check against.
{
    return boundary.x * tree.y - boundary.y * tree.x;
}

bool IsVisible(const ATreePosition &tree, const float &viewAngle, const float &direction)
{
    if (IsLeft(tree, GetBorderPosition(direction+viewAngle/2)) <= 0 &&
            IsLeft(tree, GetBorderPosition(direction-viewAngle/2)) >= 0)
        return true;
    else
        return false;
}

float GetAngle(const ATreePosition &tree) // Missing something here. Math doesn't add up.
{
    return atan2(tree.y, tree.x);
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

float GetAngleWhereISeeTheMaxNumberOfTrees(float angleOfViewInDegree, ATreePosition* forest, int forestSize)
{
    float facingAngle = 0.F; // In radians.
    float optimalAngle = 0.F;
    int mostTrees = 0;
    float angleOfViewInRadian = DegreesToRadian(angleOfViewInDegree);

    for (int i=0; i<forestSize; i++)
    {
        facingAngle = GetAngle(forest[i]);

        int count = 0;
        for (int j=0; j<forestSize; j++)
        {
            if(i != j && IsVisible(forest[j], angleOfViewInRadian, facingAngle))
            {
                cout << "Tree " << j << " visible from " << i << endl;
                count++;
            }
        }
        cout << "Number of trees " << count << endl;
        if(count > mostTrees)
        {
            cout << "Angle changed from " << RadianToDegrees(optimalAngle) << " to " << RadianToDegrees(facingAngle) << " because " << mostTrees << " < " << count << endl;
            mostTrees = count;
            optimalAngle = facingAngle;
        }
    }
    return optimalAngle;
}

void PrintList(ATreePosition *forest, int forestSize)
{
    for(int i=0; i<forestSize; i++)
        cout << "i: " << i << "\t" << forest[i].x << "\t" << forest[i].y << endl;
}

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    QBrush greenBrush(Qt::green);
    QBrush blueBrush(Qt::blue);
    QBrush redBrush(Qt::red);
    QPen outlinePen(Qt::black);
    QPen facing(Qt::green);
    QPen trees(Qt::red);
    outlinePen.setWidth(2);

    scene->setSceneRect(static_cast<double>(-viewDistance), static_cast<double>(-viewDistance), static_cast<double>(2*viewDistance), static_cast<double>(2*viewDistance));

    //scene->addEllipse(GetBorderPosition(0).x-2.5, GetBorderPosition(0).y-2.5, 5, 5, outlinePen, redBrush);

    scene->addEllipse(-2.5, -2.5, 5, 5, outlinePen, blueBrush);
    scene->addEllipse(static_cast<double>(-viewDistance)*1.2, static_cast<double>(-viewDistance)*1.2, static_cast<double>(viewDistance)*2.4, static_cast<double>(viewDistance)*2.4, outlinePen);

    ATreePosition my_list[num_trees];
    for (int i=0; i< num_trees; i++)
    {
        my_list[i] = ATreePosition(rand()%static_cast<int>(viewDistance*2)-viewDistance, rand()%static_cast<int>(viewDistance*2)-viewDistance);
        scene->addEllipse(static_cast<double>(my_list[i].x)-2.5, static_cast<double>(my_list[i].y)-2.5, 5, 5, outlinePen, greenBrush);
        scene->addText(QString::number(i))->setPos(static_cast<double>(my_list[i].x), static_cast<double>(my_list[i].y));
    }

    float viewAngle = 45.F;
    float angle = GetAngleWhereISeeTheMaxNumberOfTrees(viewAngle, my_list, num_trees);    // Radian
    ui->lineEdit->setText(QString::number(static_cast<double>(180.F + RadianToDegrees(angle))));
    scene->addLine(0, 0, static_cast<double>(viewDistance * cos(DegreesToRadian(viewAngle/2)+angle)), static_cast<double>(viewDistance * sin(DegreesToRadian(viewAngle/2)+angle)));
    scene->addLine(0, 0, static_cast<double>(viewDistance * cos(DegreesToRadian(-viewAngle/2)+angle)), static_cast<double>(viewDistance * sin(DegreesToRadian(-viewAngle/2)+angle)));
    scene->addLine(0, 0, static_cast<double>(viewDistance * cos(angle)), static_cast<double>(viewDistance * sin(angle)), facing);
}

Dialog::~Dialog()
{
    delete ui;
}

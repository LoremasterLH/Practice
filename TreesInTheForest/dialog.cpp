#include "dialog.h"
#include "ui_dialog.h"
#include <iostream>
#include <list>
#include <math.h>
#include <random>

using namespace std;

const float viewDistance = 200.0F; // Assuming distance of view.
const float PI = 3.14159265358979323846F;
const int num_trees = 30;

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
    cout << tree.y << " " << tree.x << endl;
    cout << asin(tree.y/viewDistance) << " " << acos(tree.x/viewDistance) << endl;
    return asin(tree.y/viewDistance);
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
            if(IsVisible(forest[j], angleOfViewInRadian, facingAngle))
                count++;
        }
        if(count > mostTrees)
        {
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

    scene->setSceneRect(-viewDistance, -viewDistance, 2*viewDistance, 2*viewDistance);

    //scene->addEllipse(GetBorderPosition(0).x-2.5, GetBorderPosition(0).y-2.5, 5, 5, outlinePen, redBrush);

    scene->addEllipse(-2.5, -2.5, 5, 5, outlinePen, blueBrush);
    scene->addEllipse(-viewDistance*1.2, -viewDistance*1.2, 2*viewDistance*1.2, 2*viewDistance*1.2, outlinePen);

    ATreePosition my_list[num_trees];
    for (int i=0; i< num_trees; i++)
    {
        my_list[i] = ATreePosition(rand()%(int)(viewDistance*2)-viewDistance, rand()%(int)(viewDistance*2)-viewDistance);
        scene->addEllipse(my_list[i].x-2.5, my_list[i].y-2.5, 5, 5, outlinePen, greenBrush);
    }

    double viewAngle = 110.F;
    double angle = GetAngleWhereISeeTheMaxNumberOfTrees(viewAngle, my_list, 10);    // Radian
    ui->lineEdit->setText(QString::number(RadianToDegrees(angle)));
    scene->addLine(0,0, viewDistance * cos(DegreesToRadian(viewAngle/2)+angle), viewDistance * sin(DegreesToRadian(viewAngle/2)+angle));
    scene->addLine(0,0, viewDistance * cos(DegreesToRadian(-viewAngle/2)+angle),viewDistance * sin(DegreesToRadian(-viewAngle/2)+angle));
    scene->addLine(0, 0, viewDistance * cos(angle), viewDistance * sin(angle), facing);
    delete my_list;
}

Dialog::~Dialog()
{
    delete ui;
}

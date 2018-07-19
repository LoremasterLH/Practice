#include "dialog.h"
#include "ui_dialog.h"
#include "tree.cpp"
#include <iostream>
#include <list>

using namespace std;

float GetAngleWhereISeeTheMaxNumberOfTrees(float angleOfViewInDegree, Tree* forest, int forestSize)
{
    float facingAngle = 0.F; // In radians.
    float optimalAngle = 0.F;
    int mostTrees = 0;
    float angleOfViewInRadian = DegreesToRadian(angleOfViewInDegree);

    for (int i=0; i<forestSize; i++)
    {
        facingAngle = GetAngle(forest[i], angleOfViewInRadian);

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

void PrintList(Tree *forest, int forestSize)
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

    scene->addEllipse(-2.5, -2.5, 5, 5, outlinePen, blueBrush);
    scene->addEllipse(static_cast<double>(-viewDistance)*1.2, static_cast<double>(-viewDistance)*1.2, static_cast<double>(viewDistance)*2.4, static_cast<double>(viewDistance)*2.4, outlinePen);

    Tree my_list[num_trees];
    for (int i=0; i< num_trees; i++)
    {
        my_list[i] = Tree(rand()%static_cast<int>(viewDistance*2)-viewDistance, rand()%static_cast<int>(viewDistance*2)-viewDistance);
        scene->addEllipse(static_cast<double>(my_list[i].x)-2.5, static_cast<double>(my_list[i].y)-2.5, 5, 5, outlinePen, greenBrush);
        scene->addText(QString::number(i))->setPos(static_cast<double>(my_list[i].x), static_cast<double>(my_list[i].y));
    }

    float viewAngle = 60.F;
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

#pragma once

#include <Wt/WCompositeWidget>

/*
what is this good for?
introduce the user into the wallservice
give overview of possibilities with a diagram
let the user create a gxs-id
let the user create a wall for a existing gxs-id
user has to do two things:
1) create a new gxs-id/wall
2) add neighbor nodes
*/
class FirstStepsWidget: public Wt::WCompositeWidget{
public:
    FirstStepsWidget(Wt::WContainerWidget* parent = 0);
};


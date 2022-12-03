/*  MainWindow for StellarSolver Tester Application, developed by Robert Lancaster, 2020

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QUuid>
#include <QDebug>
#include <QImageReader>
#include <QTableWidgetItem>
#include <QPainter>
#include <QDesktopServices>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#ifndef _MSC_VER
#include <sys/time.h>
#include <libgen.h>
#include <getopt.h>
#include <dirent.h>
#endif

#include <time.h>

#include <assert.h>

#include <QShortcut>
#include <QLabel>
#include <QThread>
#include <QInputDialog>
#include <QtConcurrent>
#include <QToolTip>
#include <QtGlobal>
#include "qcustomplot.h"
#include "version.h"
#include "analysis.h"


MainWindow::MainWindow() :
    QMainWindow(),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //ui->toolsWidget->setIconSize(QSize(48, 48));
    int index = 0;
    QString label = "Periodic Error Analysis";
    QWidget *widget = new Analysis(); ///QLabel(label, this);
    ui->toolsWidget->insertTab(index, widget, QIcon(), label);
    ui->toolsWidget->setTabText(index, label);
    ui->toolsWidget->setTabEnabled(index, true);
    index++;

    label = "Periodic Error Correction";
    widget = new QLabel(label, this);
    ui->toolsWidget->insertTab(index, widget, QIcon(), label);
    ui->toolsWidget->setTabText(index, label);
    ui->toolsWidget->setTabEnabled(index, true);
    index++;

    label = "PEC Curve Arithmatic";
    widget = new QLabel(label, this);
    ui->toolsWidget->insertTab(index, widget, QIcon(), label);
    ui->toolsWidget->setTabText(index, label);
    ui->toolsWidget->setTabEnabled(index, true);
    index++;

    label = "Frequency Spectrum";
    widget = new QLabel(label, this);
    ui->toolsWidget->insertTab(index, widget, QIcon(), label);
    ui->toolsWidget->setTabText(index, label);
    ui->toolsWidget->setTabEnabled(index, true);
    index++;

    label = "Simulation";
    widget = new QLabel(label, this);
    ui->toolsWidget->insertTab(index, widget, QIcon(), label);
    ui->toolsWidget->setTabText(index, label);
    ui->toolsWidget->setTabEnabled(index, true);
    index++;

    //ui->toolsWidget->tabBar()->setTabToolTip(0, QString("Setup"));
    //ui->toolsWidget->tabBar()->setTabToolTip(1, QString("Scheduler"));
    //ui->toolsWidget->tabBar()->setTabToolTip(2, QString("Analyze"));

    this->show();

}

void MainWindow::settingJustChanged()
{
}

void MainWindow::reloadConvTable()
{
}

void MainWindow::loadOptionsProfile()
{
}

MainWindow::~MainWindow()
{
    delete ui;
}

//This method clears the stars and star displays
void MainWindow::clearStars()
{
}

//This method clears the tables and displays when the user requests it.
void MainWindow::clearResults()
{
}

//These methods are for the logging of information to the textfield at the bottom of the window.
void MainWindow::logOutput(QString text)
{
}

void MainWindow::toggleFullScreen()
{
}

void MainWindow::toggleLogDisplay()
{
}

void MainWindow::helpPopup()
{
}

void MainWindow::setSubframe()
{
}

void MainWindow::startProcessMonitor()
{
}

void MainWindow::stopProcessMonitor()
{
}

//I wrote this method because we really want to do this before all 4 processes
//It was getting repetitive.
bool MainWindow::prepareForProcesses()
{
    return true;
}

//I wrote this method to display the table after star extraction has occured.
void MainWindow::displayTable()
{
}

//This method is intended to load a list of the index files to display as feedback to the user.
void MainWindow::loadIndexFilesListInFolder()
{
}

//This loads and updates the list of files we will use, if not autoindexing.
void MainWindow::loadIndexFilesToUse()
{
}

//The following methods are meant for starting the star extractor and image solver.
//The methods run when the buttons are clicked.  They call the methods inside StellarSolver and ExternalExtractorSovler

//This method responds when the user clicks the Star Extraction Button
void MainWindow::extractButtonClicked()
{
}

//This method responds when the user clicks the Star Extraction Button
void MainWindow::solveButtonClicked()
{
}

void MainWindow::extractImage()
{
}

//This method runs when the user clicks the Solve buttton
void MainWindow::solveImage()
{
}

//This sets up the External SExtractor and Solver and sets settings specific to them
void MainWindow::setupExternalExtractorSolverIfNeeded()
{
}

void MainWindow::setupStellarSolverParameters()
{
}



//This runs when the star extractor is complete.
//It reports the time taken, prints a message, loads the star extraction stars to the startable, and adds the star extraction stats to the results table.
bool MainWindow::extractorComplete()
{
    return true;
}

//This runs when the solver is complete.  It reports the time taken, prints a message, and adds the solution to the results table.
bool MainWindow::solverComplete()
{
    return true;
}

//This method will attempt to abort the star extractor, sovler, and any other processes currently being run, no matter which type
void MainWindow::abort()
{
}

//This method is meant to clear out the Astrometry settings that should change with each image
//They might be loaded from a fits file, but if the file doesn't contain them, they should be cleared.
void MainWindow::clearAstrometrySettings()
{
}

//The following methods deal with the loading and displaying of the image

//I wrote this method to select the file name for the image and call the methods in the image loader to load it
bool MainWindow::imageLoad()
{
    return false;
}

bool MainWindow::imageSave()
{
    return true;
}

void adjustScrollBar(QScrollBar *scrollBar, double factor)
{
}

void slideScrollBar(QScrollBar *scrollBar, double factor)
{
}

void MainWindow::panLeft()
{
}

void MainWindow::panRight()
{
}

void MainWindow::panUp()
{
}

void MainWindow::panDown()
{
}

//This method reacts when the user clicks the zoom in button
void MainWindow::zoomIn()
{
}

//This method reacts when the user clicks the zoom out button
void MainWindow::zoomOut()
{
}

//This code was copied and pasted and modified from rescale in Fitsview in KStars
//This method reacts when the user clicks the autoscale button and is called when the image is first loaded.
void MainWindow::autoScale()
{
}



//This method is very loosely based on updateFrame in Fitsview in Kstars
//It will redraw the image when the user loads an image, zooms in, zooms out, or autoscales
//It will also redraw the image when a change needs to be made in how the circles for the stars are displayed such as highlighting one star
void MainWindow::updateImage()
{
}

//This method was copied and pasted from Fitsdata in KStars
//It clears the image buffer out.
void MainWindow::clearImageBuffers()
{
}

//I wrote this method to respond when the user's mouse is over a star
//It displays details about that particular star in a tooltip and highlights it in the image
//It also displays the x and y position of the mouse in the image and the pixel value at that spot now.
void MainWindow::mouseMovedOverImage(QPoint location)
{
}



//This function is based upon code in the method mouseMoveEvent in FITSLabel in KStars
//It is meant to get the value from the image buffer at a particular pixel location for the display
//when the mouse is over a certain pixel
QString MainWindow::getValue(int x, int y, int channel)
{
    QString stringValue = "";
    return stringValue;
}

//I wrote the is method to respond when the user clicks on a star
//It highlights the row in the star table that corresponds to that star
void MainWindow::mouseClickedInImage(QPoint location)
{
}

void MainWindow::mousePressedInImage(QPoint location)
{
}


//THis method responds to row selections in the table and higlights the star you select in the image
void MainWindow::starClickedInTable()
{
}

//This sorts the stars by magnitude for display purposes
void MainWindow::sortStars()
{
}

//This is a helper function that I wrote for the methods below
//It add a column with a particular name to the specified table
void addColumnToTable(QTableWidget *table, QString heading)
{
}

//This is a method I wrote to hide the desired columns in a table based on their name
void setColumnHidden(QTableWidget *table, QString colName, bool hidden)
{
}

//This is a helper function that I wrote for the methods below
//It sets the value of a cell in the column of the specified name in the last row in the table
bool setItemInColumn(QTableWidget *table, QString colName, QString value)
{
    return false;
}

//This copies the stars into the table
void MainWindow::updateStarTableFromList()
{
}

void MainWindow::updateHiddenStarTableColumns()
{
}



//Note: The next 3 functions are designed to work in an easily editable way.
//To add new columns to this table, just add them to the first function
//To have it fill the column when a Sextraction or Solve is complete, add it to one or both of the next two functions
//So that the column gets setup and then gets filled in.

//This method sets up the results table to start with.
void MainWindow::setupResultsTable()
{

}

//This adds a Sextraction to the Results Table
//To add, remove, or change the way certain columns are filled when a sextraction is finished, edit them here.
void MainWindow::addExtractionToTable()
{
}


//I wrote this method to hide certain columns in the Results Table if the user wants to reduce clutter in the table.
void MainWindow::updateHiddenResultsTableColumns()
{
}

//This will write the Results table to a csv file if the user desires
//Then the user can analyze the solution information in more detail to try to perfect star extractor and solver parameters
void MainWindow::saveResultsTable()
{
}

//This will write the Star table to a csv file if the user desires
//Then the user can analyze the solution information in more detail to try to analyze the stars found or try to perfect star extractor parameters
void MainWindow::saveStarTable()
{
}

void MainWindow::saveOptionsProfiles()
{
}

void MainWindow::loadOptionsProfiles()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
}

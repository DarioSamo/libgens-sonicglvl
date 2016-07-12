#pragma once

#include "ui_HKPropertyDialog.h"
#include "HKPropertyTag.h"
#include <QDialog>

class HKPropertyDialog : public QDialog, Ui_HKPropertyDialog {
	Q_OBJECT
protected:
	HKPropertyTag *tag;
	QVector<HKPropertyValue> values;
public:
	HKPropertyDialog(QWidget *parent_widget, HKPropertyTag *tag);
	QVector<HKPropertyValue> getValues();
	QString getDescription();
private slots:
	void updateKeysTableTriggered();
	void keyChangedTriggered(int, int);
	void keyDoubleClickTriggered(int, int);
	void keyMoveUpTriggered();
	void keyMoveDownTriggered();
	void newKeyTriggered();
	void deleteKeyTriggered();
};
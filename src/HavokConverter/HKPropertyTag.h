#pragma once

#include <QPair>
#include <QString>
#include <QMap>

enum HKBitwise {
	HKBitwise_SET,
	HKBitwise_OR
};

struct HKPropertyValue {
	int key, value;
	HKBitwise bitwise;
	QString name;
};

struct HKPropertyTag {
	QString tag, description;
	QVector<HKPropertyValue> values;
};
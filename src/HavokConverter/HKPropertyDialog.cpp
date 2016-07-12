#include "HKPropertyDialog.h"

HKPropertyDialog::HKPropertyDialog(QWidget *parent_widget, HKPropertyTag *tag) : QDialog(parent_widget) {
	setupUi(this);
	this->tag = tag;
	values = tag->values;

	lbl_name->setText("Name: " + tag->tag);
	te_description->setPlainText(tag->description);
	connect(pb_up, SIGNAL(released()), this, SLOT(keyMoveUpTriggered()));
	connect(pb_down, SIGNAL(released()), this, SLOT(keyMoveDownTriggered()));
	connect(pb_new, SIGNAL(released()), this, SLOT(newKeyTriggered()));
	connect(pb_delete, SIGNAL(released()), this, SLOT(deleteKeyTriggered()));
	connect(tb_keys, SIGNAL(cellChanged(int, int)), this, SLOT(keyChangedTriggered(int, int)));
	connect(tb_keys, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(keyDoubleClickTriggered(int, int)));
	updateKeysTableTriggered();
}

void HKPropertyDialog::keyMoveUpTriggered() {
	QModelIndexList indexes = tb_keys->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int select_index = indexes.first().row();
		if (select_index > 0) {
			HKPropertyValue swap = values[select_index - 1];
			values[select_index - 1] = values[select_index];
			values[select_index] = swap;

			updateKeysTableTriggered();
			tb_keys->selectRow(select_index - 1);
		}
	}
}

void HKPropertyDialog::keyMoveDownTriggered() {
	QModelIndexList indexes = tb_keys->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int select_index = indexes.first().row();
		if (select_index < values.size() - 1) {
			HKPropertyValue swap = values[select_index + 1];
			values[select_index + 1] = values[select_index];
			values[select_index] = swap;

			updateKeysTableTriggered();
			tb_keys->selectRow(select_index + 1);
		}
	}
}

void HKPropertyDialog::newKeyTriggered() {
	HKPropertyValue value;
	value.key = 0;
	value.value = 0;
	value.bitwise = HKBitwise_SET;

	QModelIndexList indexes = tb_keys->selectionModel()->selection().indexes();
	int select_index = 0;
	if (!indexes.isEmpty()) {
		select_index = indexes.first().row();
		values.insert(select_index, value);
	}
	else {
		select_index = values.size();
		values.append(value);
	}

	updateKeysTableTriggered();
	tb_keys->selectRow(select_index);
}

void HKPropertyDialog::deleteKeyTriggered() {
	QModelIndexList indexes = tb_keys->selectionModel()->selection().indexes();
	if (!indexes.isEmpty()) {
		int index = indexes.first().row();
		values.remove(index);
		updateKeysTableTriggered();
	}
}

void HKPropertyDialog::keyChangedTriggered(int row, int column) {
	if (column < 2) {
		QTableWidgetItem *item = tb_keys->item(row, column);
		int item_v = item->text().toInt();
		if (column == 0)
			values[row].key = item_v;
		else
			values[row].value = item_v;
	}
}

void HKPropertyDialog::keyDoubleClickTriggered(int row, int column) {
	if (column == 2) {
		QTableWidgetItem *item = tb_keys->item(row, column);
		bool is_set = item->text() == "SET";
		item->setText(is_set ? "OR" : "SET");
		values[row].bitwise = is_set ? HKBitwise_OR : HKBitwise_SET;
	}
}

QVector<HKPropertyValue> HKPropertyDialog::getValues() {
	return values;
}

QString HKPropertyDialog::getDescription() {
	return te_description->toPlainText();
}

void HKPropertyDialog::updateKeysTableTriggered() {
	tb_keys->clearContents();
	int key_size = values.size();
	tb_keys->setRowCount(key_size);
	
	int row = 0;
	foreach(const HKPropertyValue &value, values) {
		QTableWidgetItem *key_item = new QTableWidgetItem(QString("%1").arg(value.key));
		QTableWidgetItem *value_item = new QTableWidgetItem(QString("%1").arg(value.value));
		QTableWidgetItem *bitwise_item = new QTableWidgetItem(QString("%1").arg(value.bitwise == HKBitwise_SET ? "SET" : "OR"));
		key_item->setToolTip("key");
		value_item->setToolTip("value");
		bitwise_item->setTextAlignment(Qt::AlignHCenter);
		bitwise_item->setToolTip("bitwise");
		bitwise_item->setFlags(value_item->flags() ^ Qt::ItemIsEditable);
		tb_keys->setItem(row, 0, key_item);
		tb_keys->setItem(row, 1, value_item);
		tb_keys->setItem(row, 2, bitwise_item);
		tb_keys->setRowHeight(row, 20);
		row++;
	}
}
#pragma once

#include <QLabel>

struct ClickableLabel : QLabel {
	Q_OBJECT
protected:
	void mousePressEvent(QMouseEvent *e) override {
		emit clicked();
		QLabel::mousePressEvent(e);
	}
signals:
	void clicked();
};

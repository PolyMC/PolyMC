#pragma once

#include <QDialog>

namespace Ui {
class ReviewMessageBox;
}

class ReviewMessageBox : public QDialog {
    Q_OBJECT

public:
    static ReviewMessageBox* create(QWidget* parent, QString&& title, QString&& icon = "");

    using ModInformation = struct {
        QString name;
        QString filename;
    };

    void appendMod(ModInformation&& info);
    QStringList deselectedMods();

    void setDescription(const QString &desc);
    void setCheckLabel(const QString &desc);

    ~ReviewMessageBox();

protected:
    ReviewMessageBox(QWidget* parent, const QString& title, const QString& icon);

    Ui::ReviewMessageBox* ui;
};

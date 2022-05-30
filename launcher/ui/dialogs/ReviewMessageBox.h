#pragma once

#include <QDialog>

namespace Ui {
class ReviewMessageBox;
}

class ReviewMessageBox : public QDialog {
    Q_OBJECT

   public:
    static auto create(QWidget* parent, QString&& title, QString&& icon = "") -> ReviewMessageBox*;

    using ModInformation = struct {
        QString name;  
        QString filename;  
    };

    void appendMod(ModInformation&& info);
    auto deselectedMods() -> QStringList;

    ~ReviewMessageBox();

   protected:
    ReviewMessageBox(QWidget* parent, const QString& title, const QString& icon);

    Ui::ReviewMessageBox* ui;
};

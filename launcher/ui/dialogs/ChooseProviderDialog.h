#pragma once

#include <QButtonGroup>
#include <QDialog>
#include <QList>
#include <QString>

#include "modplatform/ModIndex.h"
#include "modplatform/modrinth/ModrinthSource.h"

namespace Ui
{
class ChooseProviderDialog;
}

class Mod;
class NetJob;
class ModUpdateDialog;

class ChooseProviderDialog : public QDialog
{
    Q_OBJECT

    struct Response
    {
        bool skip_all = false;
        bool confirm_all = false;

        bool try_others = false;

        ModPlatform::Provider chosen;
        // Only used with Modrinth
        Modrinth::Source source;
    };

    struct Choice
    {
        ModPlatform::Provider provider;
        Modrinth::Source source;
    };

public:
    explicit ChooseProviderDialog(QWidget* parent, bool single_choice = false,
                                  bool allow_skipping = true);
    ~ChooseProviderDialog();

    Response getResponse() const
    {
        return m_response;
    }

    void setDescription(QString desc);

private slots:
    void skipOne();
    void skipAll();
    void confirmOne();
    void confirmAll();

private:
    void addProviders();
    void disableInput();

    Choice getSelectedChoice() const;

private:
    Ui::ChooseProviderDialog* ui;

    QButtonGroup m_providers;
    // Indexed by the IDs of the buttons in m_providers
    QList<Choice> m_choices;

    Response m_response;
};

#include "CustomTheme.h"
#include <QDir>
#include <FileSystem.h>
#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>

const char* themeFile = "theme.json";
const char* styleFile = "themeStyle.css";

static bool readThemeJson(const QString& path, QPalette& palette, double& fadeAmount, QColor& fadeColor, QString& name, QString& widgets)
{
    std::string path_str = path.toStdString();
    if(std::filesystem::exists(path_str) && std::filesystem::is_regular_file(path_str))
    {
        try
        {
            const nlohmann::json& root = nlohmann::json::parse(std::ifstream(path_str));

            name = root["name"].get<std::string>().c_str();
            widgets = root["widgets"].get<std::string>().c_str();

            const nlohmann::json& colorsRoot = root["colors"];
            auto readColor = [&](const char* colorName) -> QColor
            {
                auto colorValue = colorsRoot[colorName].get<std::string>().c_str();
                if(colorValue[0] != '\0')
                {
                    QColor color(colorValue);
                    if(!color.isValid())
                    {
                        qWarning() << "Color value" << colorValue << "for" << colorName << "was not recognized.";
                        return QColor();
                    }
                    return color;
                }
                return QColor();
            };
            auto readAndSetColor = [&](QPalette::ColorRole role, const char* colorName)
            {
                auto color = readColor(colorName);
                if(color.isValid())
                {
                    palette.setColor(role, color);
                }
                else
                {
                    qDebug() << "Color value for" << colorName << "was not present.";
                }
            };

            // palette
            readAndSetColor(QPalette::Window, "Window");
            readAndSetColor(QPalette::WindowText, "WindowText");
            readAndSetColor(QPalette::Base, "Base");
            readAndSetColor(QPalette::AlternateBase, "AlternateBase");
            readAndSetColor(QPalette::ToolTipBase, "ToolTipBase");
            readAndSetColor(QPalette::ToolTipText, "ToolTipText");
            readAndSetColor(QPalette::Text, "Text");
            readAndSetColor(QPalette::Button, "Button");
            readAndSetColor(QPalette::ButtonText, "ButtonText");
            readAndSetColor(QPalette::BrightText, "BrightText");
            readAndSetColor(QPalette::Link, "Link");
            readAndSetColor(QPalette::Highlight, "Highlight");
            readAndSetColor(QPalette::HighlightedText, "HighlightedText");

            //fade
            fadeColor = readColor("fadeColor");
            fadeAmount = colorsRoot.value("fadeAmount", 0.5);

        }
        catch (const std::exception& e)
        {
            qWarning() << "Couldn't load theme json: " << e.what();
            return false;
        }
    }
    else
    {
        qDebug() << "No theme json present.";
        return false;
    }
    return true;
}

static bool writeThemeJson(const QString& path, const QPalette& palette, double fadeAmount, const QColor& fadeColor, QString name, QString widgets)
{
    nlohmann::json rootObj;
    rootObj["name"] = name.toStdString();
    rootObj["widgets"] = widgets.toStdString();

    nlohmann::json colorsObj;
    auto insertColor = [&](QPalette::ColorRole role, const char* colorName)
    {
        colorsObj[colorName] = palette.color(role).name().toStdString();
    };

    // palette
    insertColor(QPalette::Window, "Window");
    insertColor(QPalette::WindowText, "WindowText");
    insertColor(QPalette::Base, "Base");
    insertColor(QPalette::AlternateBase, "AlternateBase");
    insertColor(QPalette::ToolTipBase, "ToolTipBase");
    insertColor(QPalette::ToolTipText, "ToolTipText");
    insertColor(QPalette::Text, "Text");
    insertColor(QPalette::Button, "Button");
    insertColor(QPalette::ButtonText, "ButtonText");
    insertColor(QPalette::BrightText, "BrightText");
    insertColor(QPalette::Link, "Link");
    insertColor(QPalette::Highlight, "Highlight");
    insertColor(QPalette::HighlightedText, "HighlightedText");

    // fade
    colorsObj["fadeColor"] = fadeColor.name().toStdString();
    colorsObj["fadeAmount"] = fadeAmount;

    rootObj["colors"] = colorsObj;
    try
    {
        std::ofstream file(path.toStdString());
        file << rootObj.dump(4);
        file.close();
        return true;
    }
    catch (const std::exception& e)
    {
        qWarning() << "Failed to write theme json to" << path;
        return false;
    }
}

CustomTheme::CustomTheme(ITheme* baseTheme, QString folder)
{
    m_id = folder;
    QString path = FS::PathCombine("themes", m_id);
    QString pathResources = FS::PathCombine("themes", m_id, "resources");

    qDebug() << "Loading theme" << m_id;

    if(!FS::ensureFolderPathExists(path) || !FS::ensureFolderPathExists(pathResources))
    {
        qWarning() << "couldn't create folder for theme!";
        m_palette = baseTheme->colorScheme();
        m_styleSheet = baseTheme->appStyleSheet();
        return;
    }

    auto themeFilePath = FS::PathCombine(path, themeFile);

    m_palette = baseTheme->colorScheme();
    if (!readThemeJson(themeFilePath, m_palette, m_fadeAmount, m_fadeColor, m_name, m_widgets))
    {
        m_name = "Custom";
        m_palette = baseTheme->colorScheme();
        m_fadeColor = baseTheme->fadeColor();
        m_fadeAmount = baseTheme->fadeAmount();
        m_widgets = baseTheme->qtTheme();

        QFileInfo info(themeFilePath);
        if(!info.exists())
        {
            writeThemeJson(themeFilePath, m_palette, m_fadeAmount, m_fadeColor, "Custom", m_widgets);
        }
    }
    else
    {
        m_palette = fadeInactive(m_palette, m_fadeAmount, m_fadeColor);
    }

    auto cssFilePath = FS::PathCombine(path, styleFile);
    QFileInfo info (cssFilePath);
    if(info.isFile())
    {
        try
        {
            // TODO: validate css?
            m_styleSheet = QString::fromUtf8(FS::read(cssFilePath));
        }
        catch (const Exception &e)
        {
            qWarning() << "Couldn't load css:" << e.cause() << "from" << cssFilePath;
            m_styleSheet = baseTheme->appStyleSheet();
        }
    }
    else
    {
        qDebug() << "No theme css present.";
        m_styleSheet = baseTheme->appStyleSheet();
        try
        {
            FS::write(cssFilePath, m_styleSheet.toUtf8());
        }
        catch (const Exception &e)
        {
            qWarning() << "Couldn't write css:" << e.cause() << "to" << cssFilePath;
        }
    }
}

QStringList CustomTheme::searchPaths()
{
    return { FS::PathCombine("themes", m_id, "resources") };
}


QString CustomTheme::id()
{
    return m_id;
}

QString CustomTheme::name()
{
    return m_name;
}

bool CustomTheme::hasColorScheme()
{
    return true;
}

QPalette CustomTheme::colorScheme()
{
    return m_palette;
}

bool CustomTheme::hasStyleSheet()
{
    return true;
}

QString CustomTheme::appStyleSheet()
{
    return m_styleSheet;
}

double CustomTheme::fadeAmount()
{
    return m_fadeAmount;
}

QColor CustomTheme::fadeColor()
{
    return m_fadeColor;
}

QString CustomTheme::qtTheme()
{
    return m_widgets;
}

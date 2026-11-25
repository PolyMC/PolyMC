#include "CustomTheme.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <Json.h>
#include <FileSystem.h>
#include <Exception.h>

const char* themeFile = "theme.json";
const char* styleFile = "themeStyle.css";

namespace
{
QColor readColor(const QJsonObject& colors, QString key)
{
    auto colorValue = Json::ensureString(colors, key, QString());
    if (!colorValue.isEmpty())
    {
        QColor color(colorValue);
        if (!color.isValid())
        {
            qWarning() << "Color value" << colorValue << "for" << key
                       << "was not recognized.";
            return QColor();
        }
        return color;
    }
    return QColor();
}

void readAndSetColor(const QJsonObject& colors, QPalette& palette,
                     QPalette::ColorRole role, QString key)
{
    auto color = readColor(colors, key);
    if (color.isValid())
    {
        palette.setColor(role, color);
    }
    else
    {
        qDebug() << "Color value for" << key << "was not present.";
    }
}

void insertColor(QJsonObject& colors, const QPalette& palette,
                 QPalette::ColorRole role, QString key)
{
    colors.insert(key, palette.color(role).name());
}
}  // namespace

static bool readThemeJson(const QString& path, QPalette& palette,
                          double& fadeAmount, QColor& fadeColor, QString& name,
                          QString& widgets)
{
    QFileInfo pathInfo(path);
    if (pathInfo.exists() && pathInfo.isFile())
    {
        try
        {
            auto doc = Json::requireDocument(path, "Theme JSON file");
            const QJsonObject root = doc.object();
            name = Json::requireString(root, "name", "Theme name");
            widgets = Json::requireString(root, "widgets", "Qt widget theme");
            auto colorsRoot =
                Json::requireObject(root, "colors", "colors object");
            // palette
            readAndSetColor(colorsRoot, palette, QPalette::Window, "Window");
            readAndSetColor(colorsRoot, palette, QPalette::WindowText,
                            "WindowText");
            readAndSetColor(colorsRoot, palette, QPalette::Base, "Base");
            readAndSetColor(colorsRoot, palette, QPalette::AlternateBase,
                            "AlternateBase");
            readAndSetColor(colorsRoot, palette, QPalette::ToolTipBase,
                            "ToolTipBase");
            readAndSetColor(colorsRoot, palette, QPalette::ToolTipText,
                            "ToolTipText");
            readAndSetColor(colorsRoot, palette, QPalette::Text, "Text");
            readAndSetColor(colorsRoot, palette, QPalette::Button, "Button");
            readAndSetColor(colorsRoot, palette, QPalette::ButtonText,
                            "ButtonText");
            readAndSetColor(colorsRoot, palette, QPalette::BrightText,
                            "BrightText");
            readAndSetColor(colorsRoot, palette, QPalette::Link, "Link");
            readAndSetColor(colorsRoot, palette, QPalette::Highlight,
                            "Highlight");
            readAndSetColor(colorsRoot, palette, QPalette::HighlightedText,
                            "HighlightedText");

            // fade
            fadeColor = readColor(colorsRoot, "fadeColor");
            fadeAmount = Json::ensureDouble(colorsRoot, "fadeAmount", 0.5);
        }
        catch (const Exception& e)
        {
            qWarning() << "Couldn't load theme json: " << e.cause();
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

static bool writeThemeJson(const QString& path, const QPalette& palette,
                           double fadeAmount, QColor fadeColor, QString name,
                           QString widgets)
{
    QJsonObject rootObj;
    rootObj.insert("name", name);
    rootObj.insert("widgets", widgets);

    QJsonObject colorsObj;
    // palette
    insertColor(colorsObj, palette, QPalette::Window, "Window");
    insertColor(colorsObj, palette, QPalette::WindowText, "WindowText");
    insertColor(colorsObj, palette, QPalette::Base, "Base");
    insertColor(colorsObj, palette, QPalette::AlternateBase, "AlternateBase");
    insertColor(colorsObj, palette, QPalette::ToolTipBase, "ToolTipBase");
    insertColor(colorsObj, palette, QPalette::ToolTipText, "ToolTipText");
    insertColor(colorsObj, palette, QPalette::Text, "Text");
    insertColor(colorsObj, palette, QPalette::Button, "Button");
    insertColor(colorsObj, palette, QPalette::ButtonText, "ButtonText");
    insertColor(colorsObj, palette, QPalette::BrightText, "BrightText");
    insertColor(colorsObj, palette, QPalette::Link, "Link");
    insertColor(colorsObj, palette, QPalette::Highlight, "Highlight");
    insertColor(colorsObj, palette, QPalette::HighlightedText,
                "HighlightedText");

    // fade
    colorsObj.insert("fadeColor", fadeColor.name());
    colorsObj.insert("fadeAmount", fadeAmount);

    rootObj.insert("colors", colorsObj);
    try
    {
        Json::write(rootObj, path);
        return true;
    }
    catch (const Exception&)
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

    if (!FS::ensureFolderPathExists(path) ||
        !FS::ensureFolderPathExists(pathResources))
    {
        qWarning() << "couldn't create folder for theme!";
        m_palette = baseTheme->colorScheme();
        m_styleSheet = baseTheme->appStyleSheet();
        return;
    }

    auto themeFilePath = FS::PathCombine(path, themeFile);

    m_palette = baseTheme->colorScheme();
    if (!readThemeJson(themeFilePath, m_palette, m_fadeAmount, m_fadeColor,
                       m_name, m_widgets))
    {
        m_name = "Custom";
        m_palette = baseTheme->colorScheme();
        m_fadeColor = baseTheme->fadeColor();
        m_fadeAmount = baseTheme->fadeAmount();
        m_widgets = baseTheme->qtTheme();

        QFileInfo info(themeFilePath);
        if (!info.exists())
        {
            writeThemeJson(themeFilePath, m_palette, m_fadeAmount, m_fadeColor,
                           "Custom", m_widgets);
        }
    }
    else
    {
        m_palette = fadeInactive(m_palette, m_fadeAmount, m_fadeColor);
    }

    auto cssFilePath = FS::PathCombine(path, styleFile);
    QFileInfo info(cssFilePath);
    if (info.isFile())
    {
        try
        {
            // TODO: validate css?
            m_styleSheet = QString::fromUtf8(FS::read(cssFilePath));
        }
        catch (const Exception& e)
        {
            qWarning() << "Couldn't load css:" << e.cause() << "from"
                       << cssFilePath;
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
        catch (const Exception& e)
        {
            qWarning() << "Couldn't write css:" << e.cause() << "to"
                       << cssFilePath;
        }
    }
}

QStringList CustomTheme::searchPaths()
{
    return {FS::PathCombine("themes", m_id, "resources")};
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

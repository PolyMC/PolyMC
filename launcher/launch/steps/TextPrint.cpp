#include "TextPrint.h"

TextPrint::TextPrint(LaunchTask *parent, const QString &line, MessageLevel::Enum level) : LaunchStep(parent), m_level(level)
{
    m_lines.append(line);
}

void TextPrint::executeTask()
{
    emit logLines(m_lines, m_level);
    emitSucceeded();
}

bool TextPrint::canAbort() const
{
    return true;
}

bool TextPrint::abort()
{
    emitFailed("Aborted.");
    return true;
}

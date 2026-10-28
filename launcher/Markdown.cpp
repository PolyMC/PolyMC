#include "Markdown.h"

#include <QObject>
#include <QRegularExpression>
#include <QUrl>

#include <md4c.h>

namespace {

struct Renderer {
    QByteArray out;
    QByteArray pendingHtml;
    int imageNesting = 0;
};

void escape(QByteArray& out, const MD_CHAR* text, MD_SIZE size)
{
    for (MD_SIZE i = 0; i < size; ++i) {
        switch (text[i]) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            default:
                out += text[i];
        }
    }
}

void attribute(QByteArray& out, const MD_ATTRIBUTE& attr)
{
    for (int i = 0; attr.substr_offsets[i] < attr.size; ++i) {
        const auto offset = attr.substr_offsets[i];
        const auto size = attr.substr_offsets[i + 1] - offset;
        switch (attr.substr_types[i]) {
            case MD_TEXT_NULLCHAR:
                out += "\xEF\xBF\xBD";
                break;
            case MD_TEXT_ENTITY:
                out.append(attr.text + offset, static_cast<int>(size));
                break;
            default:
                escape(out, attr.text + offset, size);
        }
    }
}

void flushHtml(Renderer* r)
{
    if (r->pendingHtml.isEmpty())
        return;
    r->out += Markdown::lowerHtml(QString::fromUtf8(r->pendingHtml)).toUtf8();
    r->pendingHtml.clear();
}

void openCell(QByteArray& out, const char* tag, const MD_BLOCK_TD_DETAIL* detail)
{
    out += '<';
    out += tag;
    switch (detail->align) {
        case MD_ALIGN_LEFT:
            out += " align=\"left\"";
            break;
        case MD_ALIGN_CENTER:
            out += " align=\"center\"";
            break;
        case MD_ALIGN_RIGHT:
            out += " align=\"right\"";
            break;
        default:
            break;
    }
    if (qstrcmp(tag, "th") == 0)
        out += " style=\"background-color: palette(alternate-base)\"";
    out += '>';
}

int enterBlock(MD_BLOCKTYPE type, void* detail, void* userdata)
{
    auto r = static_cast<Renderer*>(userdata);
    flushHtml(r);
    auto& out = r->out;
    switch (type) {
        case MD_BLOCK_QUOTE:
            out += "<table width=\"100%\" cellspacing=\"0\"><tr><td style=\"border-left: 3px solid palette(mid); padding-left: 8px\">";
            break;
        case MD_BLOCK_UL:
            out += "<ul>";
            break;
        case MD_BLOCK_OL: {
            const auto start = static_cast<MD_BLOCK_OL_DETAIL*>(detail)->start;
            out += start == 1 ? QByteArray("<ol>") : "<ol start=\"" + QByteArray::number(start) + "\">";
            break;
        }
        case MD_BLOCK_LI:
            out += "<li>";
            break;
        case MD_BLOCK_HR:
            out += "<hr>";
            break;
        case MD_BLOCK_H: {
            const auto level = static_cast<MD_BLOCK_H_DETAIL*>(detail)->level;
            out += level == 6 ? QByteArray("<h6 style=\"font-size: x-small; font-weight: bold\">") : "<h" + QByteArray::number(level) + ">";
            break;
        }
        case MD_BLOCK_CODE:
            out += "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"8\"><tr><td style=\"background-color: palette(alternate-base)\"><pre style=\"margin: 0\">";
            break;
        case MD_BLOCK_P:
            out += "<p>";
            break;
        case MD_BLOCK_TABLE:
            out += "<table border=\"1\" cellspacing=\"0\" cellpadding=\"5\" style=\"border-collapse: collapse; border-style: solid; border-color: palette(mid)\">";
            break;
        case MD_BLOCK_THEAD:
            out += "<thead>";
            break;
        case MD_BLOCK_TBODY:
            out += "<tbody>";
            break;
        case MD_BLOCK_TR:
            out += "<tr>";
            break;
        case MD_BLOCK_TH:
            openCell(out, "th", static_cast<MD_BLOCK_TD_DETAIL*>(detail));
            break;
        case MD_BLOCK_TD:
            openCell(out, "td", static_cast<MD_BLOCK_TD_DETAIL*>(detail));
            break;
        default:
            break;
    }
    return 0;
}

int leaveBlock(MD_BLOCKTYPE type, void* detail, void* userdata)
{
    auto r = static_cast<Renderer*>(userdata);
    flushHtml(r);
    auto& out = r->out;
    switch (type) {
        case MD_BLOCK_QUOTE:
            out += "</td></tr></table>";
            break;
        case MD_BLOCK_UL:
            out += "</ul>";
            break;
        case MD_BLOCK_OL:
            out += "</ol>";
            break;
        case MD_BLOCK_LI:
            out += "</li>";
            break;
        case MD_BLOCK_H:
            out += "</h" + QByteArray::number(static_cast<MD_BLOCK_H_DETAIL*>(detail)->level) + ">";
            break;
        case MD_BLOCK_CODE:
            out += "</pre></td></tr></table>";
            break;
        case MD_BLOCK_P:
            out += "</p>";
            break;
        case MD_BLOCK_TABLE:
            out += "</table>";
            break;
        case MD_BLOCK_THEAD:
            out += "</thead>";
            break;
        case MD_BLOCK_TBODY:
            out += "</tbody>";
            break;
        case MD_BLOCK_TR:
            out += "</tr>";
            break;
        case MD_BLOCK_TH:
            out += "</th>";
            break;
        case MD_BLOCK_TD:
            out += "</td>";
            break;
        default:
            break;
    }
    return 0;
}

int enterSpan(MD_SPANTYPE type, void* detail, void* userdata)
{
    auto r = static_cast<Renderer*>(userdata);
    if (r->imageNesting > 0) {
        if (type == MD_SPAN_IMG)
            ++r->imageNesting;
        return 0;
    }
    flushHtml(r);
    auto& out = r->out;
    switch (type) {
        case MD_SPAN_EM:
            out += "<em>";
            break;
        case MD_SPAN_STRONG:
            out += "<strong>";
            break;
        case MD_SPAN_A: {
            const auto a = static_cast<MD_SPAN_A_DETAIL*>(detail);
            out += "<a href=\"";
            attribute(out, a->href);
            if (a->title.size) {
                out += "\" title=\"";
                attribute(out, a->title);
            }
            out += "\">";
            break;
        }
        case MD_SPAN_IMG: {
            const auto img = static_cast<MD_SPAN_IMG_DETAIL*>(detail);
            out += "<img src=\"";
            attribute(out, img->src);
            if (img->title.size) {
                out += "\" title=\"";
                attribute(out, img->title);
            }
            out += "\" alt=\"";
            ++r->imageNesting;
            break;
        }
        case MD_SPAN_CODE:
            out += "<code style=\"background-color: palette(alternate-base)\">";
            break;
        case MD_SPAN_DEL:
            out += "<s>";
            break;
        default:
            break;
    }
    return 0;
}

int leaveSpan(MD_SPANTYPE type, void* /*detail*/, void* userdata)
{
    auto r = static_cast<Renderer*>(userdata);
    if (r->imageNesting > 0) {
        if (type == MD_SPAN_IMG && --r->imageNesting == 0)
            r->out += "\">";
        return 0;
    }
    flushHtml(r);
    auto& out = r->out;
    switch (type) {
        case MD_SPAN_EM:
            out += "</em>";
            break;
        case MD_SPAN_STRONG:
            out += "</strong>";
            break;
        case MD_SPAN_A:
            out += "</a>";
            break;
        case MD_SPAN_CODE:
            out += "</code>";
            break;
        case MD_SPAN_DEL:
            out += "</s>";
            break;
        default:
            break;
    }
    return 0;
}

int onText(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* userdata)
{
    auto r = static_cast<Renderer*>(userdata);
    if (r->imageNesting == 0) {
        if (type == MD_TEXT_HTML) {
            r->pendingHtml.append(text, static_cast<int>(size));
            return 0;
        }
        if (!r->pendingHtml.isEmpty() && type == MD_TEXT_SOFTBR) {
            r->pendingHtml += '\n';
            return 0;
        }
    }
    flushHtml(r);
    auto& out = r->out;
    switch (type) {
        case MD_TEXT_NULLCHAR:
            out += "\xEF\xBF\xBD";
            break;
        case MD_TEXT_BR:
            out += r->imageNesting ? " " : "<br>";
            break;
        case MD_TEXT_SOFTBR:
            out += r->imageNesting ? " " : "\n";
            break;
        case MD_TEXT_ENTITY:
            out.append(text, static_cast<int>(size));
            break;
        default:
            escape(out, text, size);
    }
    return 0;
}

QString iframeLink(const QString& attributes)
{
    static const QRegularExpression src(QStringLiteral("\\bsrc\\s*=\\s*[\"']?([^\"'\\s>]+)"), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression youtube(QStringLiteral("^https?://(?:www\\.)?youtube(?:-nocookie)?\\.com/embed/([A-Za-z0-9_-]{11})"));

    const QUrl url(src.match(attributes).captured(1).replace(QStringLiteral("&amp;"), QStringLiteral("&")));
    if (url.scheme() != "http" && url.scheme() != "https")
        return {};

    const auto video = youtube.match(url.toString());
    if (video.hasMatch())
        return "<p><a href=\"https://www.youtube.com/watch?v=" + video.captured(1) + "\">" + QObject::tr("Watch on YouTube") + "</a></p>";
    return "<p><a href=\"" + url.toString(QUrl::FullyEncoded).toHtmlEscaped() + "\">" + url.host().toHtmlEscaped() + "</a></p>";
}

}  // namespace

QString Markdown::toHtml(const QByteArray& markdown)
{
    Renderer r;
    MD_PARSER parser{};
    parser.flags = MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH | MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEEMAILAUTOLINKS;
    parser.enter_block = enterBlock;
    parser.leave_block = leaveBlock;
    parser.enter_span = enterSpan;
    parser.leave_span = leaveSpan;
    parser.text = onText;

    md_parse(markdown.constData(), static_cast<MD_SIZE>(markdown.size()), &parser, &r);
    flushHtml(&r);
    return QString::fromUtf8(r.out);
}

QString Markdown::lowerHtml(const QString& html)
{
    static const QRegularExpression tag(QStringLiteral("<(/?)(details|summary|del|strike|ins|iframe)\\b([^>]*)>"),
                                        QRegularExpression::CaseInsensitiveOption);

    QString out;
    decltype(html.size()) last = 0;
    auto it = tag.globalMatch(html);
    while (it.hasNext()) {
        const auto match = it.next();
        out += html.mid(last, match.capturedStart() - last);
        last = match.capturedEnd();

        const bool closing = !match.captured(1).isEmpty();
        const auto name = match.captured(2).toLower();
        if (name == "details")
            out += closing ? "</div>" : "<div>";
        else if (name == "summary")
            out += closing ? "</b></p>" : "<p><b>";
        else if (name == "ins")
            out += closing ? "</u>" : "<u>";
        else if (name == "iframe")
            out += closing ? QString() : iframeLink(match.captured(3));
        else
            out += closing ? "</s>" : "<s>";
    }
    out += html.mid(last);
    return out;
}

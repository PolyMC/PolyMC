#include <QLineEdit>

class FocusLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    FocusLineEdit(QWidget *parent);
    ~FocusLineEdit() override
    {
    }

protected:
    void focusInEvent(QFocusEvent *e) override;
    void mousePressEvent(QMouseEvent *me) override;

    bool _selectOnMousePress;
};

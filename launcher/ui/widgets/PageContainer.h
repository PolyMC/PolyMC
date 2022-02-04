/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <QWidget>
#include <QModelIndex>

#include "ui/pages/BasePageProvider.h"
#include "ui/pages/BasePageContainer.h"

class QLayout;
class IconLabel;
class QSortFilterProxyModel;
class PageModel;
class QLabel;
class QListView;
class QLineEdit;
class QStackedLayout;
class QGridLayout;

class PageContainer : public QWidget, public BasePageContainer
{
    Q_OBJECT
public:
    explicit PageContainer(BasePageProvider *pageProvider, QString defaultId = QString(),
                        QWidget *parent = nullptr);
    ~PageContainer() override {}

    void addButtons(QWidget * buttons);
    void addButtons(QLayout * buttons);
    /*
     * Save any unsaved state and prepare to be closed.
     * @return true if everything can be saved, false if there is something that requires attention
     */
    bool prepareToClose();
    bool saveAll();

    /* request close - used by individual pages */
    bool requestClose() override
    {
        if(m_container)
        {
            return m_container->requestClose();
        }
        return false;
    }

    bool selectPage(QString pageId) override;

    void refreshContainer() override;
    virtual void setParentContainer(BasePageContainer * container)
    {
        m_container = container;
    };

private:
    void createUI();

public slots:
    void help();

private slots:
    void currentChanged(const QModelIndex &current);
    void showPage(int row);

private:
    BasePageContainer * m_container = nullptr;
    BasePage * m_currentPage = nullptr;
    QSortFilterProxyModel *m_proxyModel;
    PageModel *m_model;
    QStackedLayout *m_pageStack;
    QListView *m_pageList;
    QLabel *m_header;
    IconLabel *m_iconHeader;
    QGridLayout *m_layout;
};

/*
 * Copyright © 2018 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// TestResultListDialog.cpp : implementation file
//
#include "TestResultListDialog.h"
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include "TestSdk.h"
#include "log.h"




CTestResultListDialog::CTestResultListDialog(QString appName, QString xml, QWidget *parent)
    : QDialog(parent),
     m_xml(xml),
     m_appName(appName)
{
    ui.setupUi(this);
    connect(ui.listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(onCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));
    connect(this, SIGNAL(runUnblockCaseCase(QString)), this, SLOT(onRunUnblockCase(QString)));
    for (int i = 0; i < CTestSdk::s_sdkCaseList.count(); i++) {
        QString name = CTestSdk::s_sdkCaseList[i];
        this->addItem(CTestSdk::s_sdkCaseList[i], "notrun", "", [=] {
            if (ui.checkBox_unblock->isChecked()) {
                initThreadMap(name);
                m_threadMap[name]->start();
            }
            else {
                CTestSdk tc;
                tc.setLogRedirect(false);
                QStringList arguments;
                arguments << "-xml" << "-vs" << "-xunitxml" << "-o" << m_xml << name;
                QTEST_SET_MAIN_SOURCE_PATH
                QTest::qExec(&tc, arguments);
                QStringList sList;
                sList.push_back(name);
                readXml(sList);
            }
        });
    }
    readXml(CTestSdk::s_sdkCaseList);
}

CTestResultListDialog::~CTestResultListDialog()
{
    clearThreadTask();
    QFile file(m_xml);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG_DEBUG("open for add error!!");
    }

    QTextStream out(&file);
    m_doc.save(out, QDomNode::EncodingFromDocument);
    file.close();
}

void CTestResultListDialog::addItem(const QString& name, const QString& result, const QString& message, function<void()> func) {
    m_mapFunc.insert(make_pair(name, func));
    QListWidgetItem *item = new QListWidgetItem(ui.listWidget, 0);
    m_mapListWidgetItem.insert(make_pair(name, item));
    QMap<QString, QVariant> qMap;
    item->setSizeHint(QSize(100, 20));
    QWidget *widget = new QWidget(ui.listWidget);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    QPushButton *pushButton = new QPushButton(widget);
    QCheckBox *checkBox = new QCheckBox(widget);
    QLabel *label = new QLabel(widget);
    qMap.insert("checkBox", (int)checkBox);
    qMap.insert("label", (int)label);
    qMap.insert("pushButton", (int)pushButton);
    qMap.insert("msg", message);
    item->setData(Qt::UserRole, qMap);
    layout->setContentsMargins(6,0,6,0);
    checkBox->setText(name);
    //checkBox->setFixedWidth(250);
    label->setText(result);
    label->setFixedWidth(50);
    pushButton->setText("run");
    pushButton->setObjectName(name);
    pushButton->setFixedWidth(50);
    connect(pushButton, SIGNAL(clicked()), this, SLOT(on_itemButton_run_clicked()));
    QString buttonStyle = QString("") +
        "QPushButton{background-color:black;color: white;   border - radius: 10px;  border: 2px groove gray;border - style: outset;}" +
        "QPushButton:hover{background-color:white; color: black;}" +
        "QPushButton:pressed{background-color:rgb(85, 170, 255);border - style: inset; }";
    pushButton->setStyleSheet(buttonStyle);
    layout->addWidget(checkBox);
    layout->addWidget(label);
    layout->addWidget(pushButton);
    layout->setSpacing(6);
    widget->setLayout(layout);
    widget->show();
    ui.listWidget->setItemWidget(item, widget);
}


void CTestResultListDialog::setResult(const QString& errors, const QString& failures, const QString& tests)
{
    ui.label_result->setText("test:" + tests + "  failures:" + failures + "  errors:" + errors);
}

void CTestResultListDialog::onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QMap<QString, QVariant> qMap = current->data(Qt::UserRole).toMap();
    QString message = qMap["msg"].toString();
    ui.textBrowser_msg->setText(message);
}

void CTestResultListDialog::on_checkBox_all_clicked()
{
    bool checked = ui.checkBox_all->isChecked();
    for (int i = 0; i < ui.listWidget->count(); i++) {
        QListWidgetItem* item = ui.listWidget->item(i);
        QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
        QCheckBox * checkBox = (QCheckBox *)(qMap["checkBox"].toInt());
        checkBox->setChecked(checked);
    }
}

void CTestResultListDialog::onRunUnblockCase(QString name)
{
    QStringList sList;
    sList << name;
    readXml(sList, "./xml/" + name + ".xml");
}

void CTestResultListDialog::initThreadMap(QString name)
{
    if (m_threadMap.find(name) == m_threadMap.end()) {
        m_threadMap[name] = new CRunCaseThread(this);
        m_threadMap[name]->setAppName(m_appName);
        m_threadMap[name]->setName(name);
        m_threadMap[name]->setParamList(QStringList() << "-xml" << "-v1" << "-xunitxml" << "-o" << ("./xml/" + name + ".xml") << name);
        m_threadMap[name]->connect(m_threadMap[name], SIGNAL(runCase(QString)), this, SLOT(onRunUnblockCase(QString)));
    }
}
void CTestResultListDialog::on_pushButton_run_clicked()
{
    if (ui.checkBox_unblock->isChecked()) {
        for (int i = 0; i < ui.listWidget->count(); i++) {
            QListWidgetItem* item = ui.listWidget->item(i);
            QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
            QCheckBox * checkBox = (QCheckBox *)(qMap["checkBox"].toInt());
            if (checkBox->isChecked()) {
                QString name = checkBox->text();
                initThreadMap(name);
                m_threadMap[name]->start();
            }
        }
    }
    else {
        QStringList sList;
        QStringList arguments;
        arguments << "-xml" << "-vs" << "-xunitxml" << "-o" << m_xml;
        for (int i = 0; i < ui.listWidget->count(); i++) {
            QListWidgetItem* item = ui.listWidget->item(i);
            QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
            QCheckBox * checkBox = (QCheckBox *)(qMap["checkBox"].toInt());
            if (checkBox->isChecked()) {
                sList.push_back(checkBox->text());
                arguments.push_back(checkBox->text());
            }
        }
        CTestSdk tc;
        tc.setLogRedirect(false);
        QTEST_SET_MAIN_SOURCE_PATH
        QTest::qExec(&tc, arguments);
        readXml(sList);
    }
    
}

void CTestResultListDialog::on_pushButton_runAll_clicked()
{
    if (ui.checkBox_unblock->isChecked()) {
        for (int i = 0; i < ui.listWidget->count(); i++) {
            QListWidgetItem* item = ui.listWidget->item(i);
            QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
            QCheckBox * checkBox = (QCheckBox *)(qMap["checkBox"].toInt());
            QString name = checkBox->text();
            initThreadMap(name);
            m_threadMap[name]->start();
        }
    }
    else {
        QStringList sList;
        QStringList arguments;
        arguments << "-xml" << "-vs" << "-xunitxml" << "-o" << m_xml;
        for (int i = 0; i < ui.listWidget->count(); i++) {
            QListWidgetItem* item = ui.listWidget->item(i);
            QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
            QCheckBox * checkBox = (QCheckBox *)(qMap["checkBox"].toInt());
            sList.push_back(checkBox->text());
            arguments.push_back(checkBox->text());
        }
        CTestSdk tc;
        tc.setLogRedirect(false);
        QTEST_SET_MAIN_SOURCE_PATH
        QTest::qExec(&tc, arguments);
        readXml(sList);
    }
}

void CTestResultListDialog::on_pushButton_clear_clicked()
{
    clearThreadTask();
    for (int i = 0; i < ui.listWidget->count(); i++) {
        QListWidgetItem* item = ui.listWidget->item(i);
        QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
        QLabel* label = (QLabel *)(qMap["label"].toInt());
        label->setText("notrun");
        qMap["msg"] = "";
        item->setData(Qt::UserRole, qMap);
    }
    m_errors = "0";
    m_failures = "0";
    m_tests = "2";
    m_doc.clear();
    ui.textBrowser_msg->clear();
    m_mapDomElement.clear();
    QDir dir("./");
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Dirs | QDir::Files))
    {
        QString strName = fileInfo.fileName();
        if (fileInfo.isDir() && (strName == "xml" || strName == "log")) {
            QDir dir(strName);
            dir.setFilter(QDir::Files);
            int fileCount = dir.count();
            for (int i = 0; i < fileCount; i++) {
                dir.remove(dir[i]);
            }
        }
    }
    updateResultLabel();
}

void CTestResultListDialog::setTestFunc(function<void()> func)
{
    m_func = func;
}

void CTestResultListDialog::readXml(QStringList& updateCaseList, QString xml)
{
    if (xml == "") {
        xml = m_xml;
    }
    QFile file(xml);
    QString error = "";
    int row = 0, column = 0;
    if (!file.open(QIODevice::ReadOnly)) return;
    QDomDocument doc;
    string sXml = xml.toStdString();
    if (!doc.setContent(&file, false, &error, &row, &column)) {
        LOG_DEBUG("parse file %s, failed:%d---%d:", sXml.c_str(), row, column, error);
        file.close();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode node = root.firstChild();
    while (!node.isNull()) {
        QDomElement element = node.toElement(); 
        if (!element.isNull()) {
            if (element.tagName() == "testcase") {
                QString name = "";
                QString result = "";
                if (element.hasAttribute("result")) {
                    result = element.attributeNode("result").value();
                }
                if (element.hasAttribute("name")) {
                    name = element.attributeNode("name").value();
                }
                int index = updateCaseList.indexOf(name);
                if (m_doc.isNull()) {
                    m_mapDomElement[name] = node;
                }
                else {
                    if (index >= 0) {
                        QDomNode newNode = node.cloneNode();
                        if (m_mapDomElement[name].isNull()) {
                            m_tests = QString::number(m_tests.toInt() + 1);
                            if (result == "fail") {
                                m_failures = QString::number(m_failures.toInt() + 1);
                            }
                            else if (result == "error") {
                                m_errors = QString::number(m_errors.toInt() + 1);
                            }
                            QDomNode refChild;
                            for (int i = CTestSdk::s_sdkCaseList.indexOf(name); i < CTestSdk::s_sdkCaseList.count(); i++) {
                                refChild = m_mapDomElement[CTestSdk::s_sdkCaseList[i]];
                                if (!refChild.isNull()) {
                                    m_doc.documentElement().insertBefore(newNode, refChild);
                                    break;
                                }
                            }
                            if (refChild.isNull()) { 
                                m_doc.documentElement().insertBefore(newNode, m_doc.documentElement().lastChild().previousSibling());
                            }
                        }
                        else {
                            m_doc.documentElement().replaceChild(newNode, m_mapDomElement[name]);
                        }
                        m_mapDomElement[name] = newNode;
                    }
                }
                
                if (index >= 0) {
                    QListWidgetItem* item = m_mapListWidgetItem[name];
                    QMap<QString, QVariant> qMap = item->data(Qt::UserRole).toMap();
                    ((QLabel *)qMap["label"].toInt())->setText(result);
                    QDomNode nodeson = element.firstChild();
                    QString message = "";
                    while (!nodeson.isNull()) {
                        QDomElement elementson = nodeson.toElement();
                        if (!elementson.isNull()) {
                            if (elementson.tagName() == "property") {
                                //
                            }
                            if (elementson.tagName() == "failure") {
                                if (elementson.hasAttribute("message")) {
                                    message = elementson.attributeNode("message").value();
                                }
                                qMap["msg"] = message;
                            }
                        }
                        nodeson = nodeson.nextSibling();
                    }
                    item->setData(Qt::UserRole, qMap);
                }
            }
        }
        node = node.nextSibling();
    }
    if (m_doc.isNull()) {
        m_doc = doc;
        m_errors = root.attributeNode("errors").value();
        m_failures = root.attributeNode("failures").value();
        m_tests = root.attributeNode("tests").value();
    }
    updateResultLabel();
}

void CTestResultListDialog::setResultXml(QString xml)
{
    m_xml = xml;
}

void CTestResultListDialog::updateResultLabel()
{
    QDomElement root = m_doc.documentElement();
    root.setAttribute("errors", m_errors);
    root.setAttribute("failures", m_failures);
    root.setAttribute("tests", m_tests);
    setResult(m_errors, m_failures, QString::number(m_tests.toInt() - 2));
}

void CTestResultListDialog::on_itemButton_run_clicked()
{
    QPushButton* item = (QPushButton*)sender();
    QString name = item->objectName();
    m_mapFunc[name]();
}

void CTestResultListDialog::setAppName(QString appName)
{
    m_appName = appName;
}

void CTestResultListDialog::clearThreadTask()
{
    for (map<QString, CRunCaseThread*>::iterator it = m_threadMap.begin(); it != m_threadMap.end(); it++) {
        CRunCaseThread* thread = it->second;
        if (thread) {
            thread->stop();
            delete thread;
            m_threadMap[it->first] = nullptr;
        }
    }
    m_threadMap.clear();
    CRunCaseThread::unlockMutexState();
}

void CTestResultListDialog::closeEvent(QCloseEvent *event)
{
    LOG_DEBUG("");
    Q_EMIT onClose();
}

void CTestResultListDialog::onClose()
{
    LOG_DEBUG("");
    //clearThreadTask();
}
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
// TestResultListDialog.h : header file
//
#pragma once

#include "ui_TestResultListDialog.h"
#include <QListWidgetItem>
#include <QDialog>
#include <QFile>
#include <QDomDocument>
#include "TestSdk.h"
#include "RunCaseThread.h"
#include "log.h"


class CTestResultListDialog :public QDialog {
    Q_OBJECT
public:
    CTestResultListDialog(QString appName, QString xml, QWidget *parent);
    ~CTestResultListDialog();
    void addItem(const QString& name, const QString& result, const QString& message, function<void()> func);
    void setResult(const QString& errors, const QString& failures, const QString& tests);
    void initThreadMap(QString name);
    void setTestFunc(function<void()> func);
    void readXml(QStringList& updateCaseList, QString xml = "");
    void setResultXml(QString xml);
    void updateResultLabel();
    void setAppName(QString appName);
    void clearThreadTask();
public Q_SLOTS:
    void onClose();
    void onCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_checkBox_all_clicked();
    void on_pushButton_run_clicked();
    void on_pushButton_runAll_clicked();
    void on_pushButton_clear_clicked();
    void on_itemButton_run_clicked();
    void onRunUnblockCase(QString name);

protected:
    void closeEvent(QCloseEvent *event);

private:
    std::map<std::string, std::function<void(std::string str)>> acc_map;
    Ui::CTestResultListDialog ui;
    QString m_xml;
    QString m_appName;
    QDomDocument m_doc;
    function<void()> m_func;
    QString m_errors;
    QString m_failures;
    QString m_tests;
    map<QString, CRunCaseThread*> m_threadMap;
    map<QString, function<void()>> m_mapFunc;
    map<QString, QListWidgetItem*> m_mapListWidgetItem;
    map<QString, QDomNode>m_mapDomElement;
};

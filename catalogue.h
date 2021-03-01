#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <QtSql>
#include <QtConcurrent>
#include "csv.h"

class CATALOGUE
{
    CSV model;
    QString qpath;
    std::wstring wpath;
    QString qname;
    QString tname;
    string sname;
    wstring wname;
    QString qyear;
    bool multi_column;
    QString qdescription;
    wstring wfile;
    QString qfile;
    wstring csv_trunk;
    vector<wstring> csv_branches;
    QVector<QString> gid_list;
    //QVector<QString> damaged_gid_list;
    QVector<QString> column_titles;
    QVector<QString> row_titles;
    QVector<QString> primary_table_column_titles;
    QString primary_table_column_template;
    QVector<QVector<QVector<int>>> tree;
    QVector<QVector<QString>> model_text_variables;
    QString csv_tables_template;  // Has '!!!' in place of the table name.
    QString ins_csv_row_template;
    int remote_control = 0;  // Directive for the object. 0 = standard, 1 = cancel.
    QVector<QString> gid_want_list;
    QVector<QVector<QVector<QString>>> thr_stmts;  // Form [thread][statement][template, bind values...]

public:
    explicit CATALOGUE() {}
    ~CATALOGUE() {}
    vector<int> jobs;
    vector<vector<int>>bot_top;
    void initialize_threading(int);
    void set_path(QString&);
    void initialize_table();
    void make_name_tree();
    string sublabelmaker(string&, QVector<QVector<int>>&);
    wstring get_csv_path(int);
    QString get_csv_branch(int);
    QString get_qname();
    string get_sname();
    string get_gid(int);
    int get_gid_size();
    string create_primary_table();
    string get_create_sub_template();
    string get_insert_csv_row_template();
    QVector<QVector<QVector<int>>> get_tree();
    QVector<QString> get_gid_list();
    void insert_primary_columns_template();
    string get_primary_template();
    void set_primary_columns_template(QString);
    void set_tree(QVector<QVector<QVector<int>>>);
    void set_gid_list(QVector<QString>);
    wstring get_csv_trunk();
    void set_csv_trunk(wstring);
    vector<wstring> get_csv_branches();
    void set_csv_branches(vector<wstring>);
    bool get_multicol();
    void set_multicol(bool);
    void set_qfile(int);
    QVector<QVector<QString>> extract_data_rows(QString&, int&);
    vector<vector<string>> extract_data_rows8(string&, int&);
    QVector<QString> get_column_titles();
    void set_column_titles(QVector<QString>);
    QVector<QString> get_row_titles();
    void set_row_titles(QVector<QString>);
    QVector<QVector<QString>> get_model_text_variables();
    QString get_year();
    void set_description(QString);
    QString get_description();
    void basic_input(QVector<QString>);
    void create_csv_tables_template();
    string get_csv_template();
    QVector<QVector<QString>> extract_text_vars(QString&);
    vector<vector<string>> extract_text_vars8(string&);
    void insert_csv_row_template();
    void print_stuff();
    void cancel_insertion();
    int get_status();
    void set_gid_want_list(QVector<QString>&);
    QVector<QString> get_gid_want_list();
    void add_statements(QVector<QString>&, int);  // NOTE: Mutexing happens within MainWindow, not Catalogue.
    QVector<QVector<QString>> take_statements(int);  // Form [statement][template, values...]. NOTE: Mutexing in MW.
    void set_wpath(wstring&);
    //void set_damaged_gid(QString&);
    //QVector<QString> get_damaged_gid_list();
};

#endif // CATALOGUE_H

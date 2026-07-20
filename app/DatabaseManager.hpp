#ifndef DATABASEMANAGER_HPP
#define DATABASEMANAGER_HPP

#include <QString>
#include <QSqlDatabase>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool init();

private:
    DatabaseManager();
    ~DatabaseManager();
    
    QSqlDatabase db;
    bool createTables();
};

#endif // DATABASEMANAGER_HPP

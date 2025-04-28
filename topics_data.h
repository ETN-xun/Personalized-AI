#ifndef TOPICS_DATA_H
#define TOPICS_DATA_H

#include <QMap>
#include <QString>
#include <QStringList>

class TopicsData {
public:
    // 获取所有话题和对应的问题列表
    static const QMap<QString, QStringList>& getAllTopics();

private:
    // 初始化话题数据（只执行一次）
    static QMap<QString, QStringList> initializeTopics();

    // 存储话题数据的静态成员变量
    static QMap<QString, QStringList> topics;
    // 保证初始化只进行一次的标志
    static bool initialized;
};

#endif // TOPICS_DATA_H
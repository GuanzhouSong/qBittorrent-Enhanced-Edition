#include "logger.h"

#include <QDateTime>
#include "base/utils/string.h"

Logger *Logger::m_instance = nullptr;

Logger::Logger()
    : m_lock(QReadWriteLock::Recursive)
    , m_msgCounter(0)
    , m_peerCounter(0)
{
}

Logger::~Logger() {}

Logger *Logger::instance()
{
    return m_instance;
}

void Logger::initInstance()
{
    if (!m_instance)
        m_instance = new Logger;
}

void Logger::freeInstance()
{
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void Logger::addMessage(const QString &message, const Log::MsgType &type)
{
    QWriteLocker locker(&m_lock);

    Log::Msg temp = {m_msgCounter++, QDateTime::currentMSecsSinceEpoch(), type, message.toHtmlEscaped()};
    m_messages.push_back(temp);

    if (m_messages.size() >= MAX_LOG_MESSAGES)
        m_messages.pop_front();

    emit newLogMessage(temp);
}

void Logger::addPeer(const QString &ip, bool blocked, const QString &reason)
{
    QWriteLocker locker(&m_lock);

    Log::Peer temp = {m_peerCounter++, QDateTime::currentMSecsSinceEpoch(), ip.toHtmlEscaped(), blocked, reason.toHtmlEscaped()};
    m_peers.push_back(temp);

    if (m_peers.size() >= MAX_LOG_MESSAGES)
        m_peers.pop_front();

    emit newLogPeer(temp);
}

QVector<Log::Msg> Logger::getMessages(int lastKnownId) const
{
    QReadLocker locker(&m_lock);

    int diff = m_msgCounter - lastKnownId - 1;
    int size = m_messages.size();

    if ((lastKnownId == -1) || (diff >= size))
        return m_messages;

    if (diff <= 0)
        return QVector<Log::Msg>();

    return m_messages.mid(size - diff);
}

QVector<Log::Peer> Logger::getPeers(int lastKnownId) const
{
    QReadLocker locker(&m_lock);

    int diff = m_peerCounter - lastKnownId - 1;
    int size = m_peers.size();

    if ((lastKnownId == -1) || (diff >= size))
        return m_peers;

    if (diff <= 0)
        return QVector<Log::Peer>();

    return m_peers.mid(size - diff);
}

void LogMsg(const QString &message, const Log::MsgType &type)
{
    Logger::instance()->addMessage(message, type);
}

#include "SystemEventHandler.h"

#ifdef Q_OS_MAC
#include "MacSystemEvents.h"
#endif

#include <QDebug>

SystemEventHandler::SystemEventHandler()
{
#ifdef Q_OS_MAC
    Q_ASSERT(!eventHandler);
    eventHandler = registerSystemHandler(this, &SystemEventHandler::triggerEvent);
#endif
}

SystemEventHandler::~SystemEventHandler()
{
#ifdef Q_OS_MAC
    Q_ASSERT(eventHandler);
    unregisterSystemHandler(eventHandler);
#endif
}

void SystemEventHandler::emitEvent(const SystemEvent event)
{
    switch (event) {
    case SCREEN_LOCKED:
        emit screenLocked();
        break;

    default:
        qCritical() << "Unknown system event:" << event;
        Q_ASSERT(false);
        break;
    }
}

void SystemEventHandler::triggerEvent(const int type, void *instance)
{
    auto *handler = static_cast<SystemEventHandler*>(instance);
    if (handler) {
        handler->emitEvent(static_cast<SystemEvent>(type));
    }
}

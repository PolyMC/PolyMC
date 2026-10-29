#pragma once

#include <QObject>
#include <QSharedPointer>

#include <functional>
#include <memory>

struct QObjectDeleteLater {
    void operator()(QObject* object) const { object->deleteLater(); }
};

/**
 * A unique pointer class with unique pointer semantics intended for derivates of QObject
 * Calls deleteLater() instead of destroying the contained object immediately
 */
template <typename T>
using unique_qobject_ptr = std::unique_ptr<T, QObjectDeleteLater>;

/**
 * A shared pointer class with shared pointer semantics intended for derivates of QObject
 * Calls deleteLater() instead of destroying the contained object immediately
 */
template <typename T>
class shared_qobject_ptr : public QSharedPointer<T> {
   public:
    constexpr shared_qobject_ptr() : QSharedPointer<T>() {}
    constexpr shared_qobject_ptr(T* ptr) : QSharedPointer<T>(ptr, &QObject::deleteLater) {}
    constexpr shared_qobject_ptr(std::nullptr_t null_ptr) : QSharedPointer<T>(null_ptr, &QObject::deleteLater) {}

    template <typename Derived>
    constexpr shared_qobject_ptr(const shared_qobject_ptr<Derived>& other) : QSharedPointer<T>(other)
    {}

    void reset() { QSharedPointer<T>::reset(); }
    void reset(const shared_qobject_ptr<T>& other)
    {
        shared_qobject_ptr<T> t(other);
        this->swap(t);
    }
};

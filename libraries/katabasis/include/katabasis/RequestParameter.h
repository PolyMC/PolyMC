#include <utility>

#pragma once

namespace Katabasis {

/// Request parameter (name-value pair) participating in authentication.
struct RequestParameter {
    RequestParameter(QByteArray n, QByteArray v): name(std::move(n)), value(std::move(v)) {}
    bool operator <(const RequestParameter &other) const {
        return (name == other.name)? (value < other.value): (name < other.name);
    }
    QByteArray name;
    QByteArray value;
};

}

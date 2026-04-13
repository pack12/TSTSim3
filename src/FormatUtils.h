// FormatUtils.h
// Shared formatting helpers for display strings.

#pragma once
#include <QString>

namespace FormatUtils {

// Formats a raw pound value into a readable string: £1.5M, £500k, £2k, etc.
inline QString formatMoney(int pounds) {
    if (pounds >= 1000000) {
        int millions = pounds / 1000000;
        int remainder = (pounds % 1000000) / 100000;
        return remainder > 0
            ? QString::fromUtf8("£%1.%2M").arg(millions).arg(remainder)
            : QString::fromUtf8("£%1M").arg(millions);
    }
    if (pounds >= 1000) {
        return QString::fromUtf8("£%1k").arg(pounds / 1000);
    }
    return QString::fromUtf8("£%1").arg(pounds);
}

} // namespace FormatUtils

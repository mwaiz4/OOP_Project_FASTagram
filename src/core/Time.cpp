#include "core/Time.h"

#include <ctime>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {

bool get_local_time(Time::seconds_type seconds, std::tm& result) {
    const std::time_t value = static_cast<std::time_t>(seconds);

#ifdef _WIN32
    return localtime_s(&result, &value) == 0;
#else
    return localtime_r(&value, &result) != nullptr;
#endif
}

} // namespace

Time::clock_fn Time::clock_source_ = nullptr;

Time::Time(seconds_type seconds)
    : seconds_(seconds) {
    if (seconds < 0) {
        throw std::invalid_argument("Time cannot be negative");
    }
}

Time Time::now() {
    if (clock_source_ != nullptr) {
        return Time(clock_source_());
    }

    const std::time_t current_time = std::time(nullptr);
    if (current_time == static_cast<std::time_t>(-1)) {
        throw std::runtime_error("Could not read the system clock");
    }

    const seconds_type seconds = static_cast<seconds_type>(current_time);
    if (seconds < 0) {
        throw std::runtime_error("The system clock returned a negative time");
    }

    return Time(seconds);
}

void Time::set_clock(clock_fn fn) noexcept {
    clock_source_ = fn;
}

Time::seconds_type Time::seconds() const noexcept {
    return seconds_;
}

std::string Time::label() const {
    const Time current = Time::now();
    const seconds_type diff = current - *this;

    if (diff < 60) {
        return "Just now";
    }
    if (diff < 3600) {
        return std::to_string(diff / 60) + " min ago";
    }
    if (diff < 86400) {
        return std::to_string(diff / 3600) + " h ago";
    }

    std::tm created_tm{};
    std::tm current_tm{};
    if (!get_local_time(seconds_, created_tm) ||
        !get_local_time(current.seconds(), current_tm)) {
        throw std::runtime_error("Could not convert time to local calendar date");
    }

    bool is_yesterday = false;
    if (current_tm.tm_yday > 0) {
        is_yesterday = created_tm.tm_year == current_tm.tm_year &&
                       created_tm.tm_yday == current_tm.tm_yday - 1;
    } else {
        // On January 1, yesterday is December 31 of the previous year.
        is_yesterday = created_tm.tm_year == current_tm.tm_year - 1 &&
                       created_tm.tm_mon == 11 && created_tm.tm_mday == 31;
    }

    if (is_yesterday) {
        return "Yesterday";
    }

    char date_buffer[32]{};
    if (std::strftime(date_buffer, sizeof(date_buffer), "%d %b", &created_tm) == 0) {
        throw std::runtime_error("Could not format time label");
    }

    std::string date_label(date_buffer);
    if (date_label.size() >= 2 && date_label[0] == '0') {
        date_label.erase(0, 1);
    }
    return date_label;
}

void Time::write(std::ostream& out) const {
    if (seconds_ > static_cast<seconds_type>(
                       (std::numeric_limits<std::int64_t>::max)())) {
        throw std::runtime_error("Time value does not fit the file format");
    }

    const std::int64_t value = static_cast<std::int64_t>(seconds_);
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    if (!out) {
        throw std::runtime_error("Failed to write Time");
    }
}

void Time::read(std::istream& in) {
    std::int64_t value = 0;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!in) {
        throw std::runtime_error("Failed to read Time");
    }
    if (value < 0) {
        throw std::runtime_error("File contains a negative Time value");
    }

    seconds_ = static_cast<seconds_type>(value);
}

Time::seconds_type operator-(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ - rhs.seconds_;
}

bool operator<(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ < rhs.seconds_;
}

bool operator>(const Time& lhs, const Time& rhs) noexcept {
    return rhs < lhs;
}

bool operator==(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ == rhs.seconds_;
}

bool operator!=(const Time& lhs, const Time& rhs) noexcept {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Time& time) {
    out << time.seconds_;
    return out;
}

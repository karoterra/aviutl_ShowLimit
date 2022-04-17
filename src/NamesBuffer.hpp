#pragma once

#include <Windows.h>

#include <string_view>
#include <tuple>

class NamesBuffer {
public:
    class Iterator {
    public:
        Iterator() : buffer_(), name_(), dir_() {
        }

        Iterator(const std::string_view& buffer)
            : buffer_(buffer)
        {
            next();
        }

        std::tuple<std::string_view, std::string_view> operator*() const {
            return { name_, dir_ };
        }

        void next() {
            size_t separator = 0, terminal = 0;
            for (size_t i = 0; i < buffer_.size(); i++) {
                if (buffer_[i] == '\0') {
                    terminal = i;
                    break;
                }
                else if (buffer_[i] == 1) {
                    separator = i;
                    continue;
                }
                if (IsDBCSLeadByte(buffer_[i]) != FALSE) {
                    i++;
                }
            }

            if (terminal == 0) {
                name_ = {};
                dir_ = {};
                buffer_ = {};
            }
            else {
                if (separator == 0) {
                    name_ = buffer_.substr(0, terminal);
                    dir_ = {};
                }
                else {
                    name_ = buffer_.substr(0, separator);
                    dir_ = buffer_.substr(separator + 1, terminal - separator - 1);
                }
                buffer_ = buffer_.substr(terminal + 1);
            }
        }

        Iterator& operator++() {
            next();
            return *this;
        }

        bool operator !=(const Iterator& other) const {
            return name_.size() != 0;
        }

    private:
        std::string_view buffer_;
        std::string_view name_;
        std::string_view dir_;
    };

    NamesBuffer(const char* buffer, size_t length)
        : buffer_(buffer, length)
    {
    }

    Iterator begin() const {
        return { buffer_ };
    }

    Iterator end() const {
        return {};
    }

private:
    std::string_view buffer_;
};

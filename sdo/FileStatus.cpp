//
// Created by bzfgottw on 23.04.15.
//

#include "FileStatus.hpp"

namespace sdo {

    void FileStatus::report(std::ostream &os, bool errors, bool warnings) const {
        if(!errors && !warnings)
            return;
        for(auto& msg : messages_) {
            if(errors && msg.error) {
                os << "Error: " << msg.msg << '\n';
                for (auto &loc : msg.locations) {
                    os << " ... at " << loc << '\n';
                }
            } else if(warnings && !msg.error) {
                os << "Warning: " << msg.msg << '\n';
                for (auto &loc : msg.locations) {
                    os << " ... at " << loc << '\n';
                }
            }
        }
    }


    std::string FileStatus::report(bool errors, bool warnings) const {
        std::ostringstream stream;
        report(stream, errors, warnings);
        return stream.str();
    }

    void FileStatus::error(const std::vector<FileLocation> &locs, std::string msg ) {
        ++num_errors_;
        messages_.emplace_back(FileMessage{true, locs, std::move(msg)});
    }

    void FileStatus::warning(const std::vector<FileLocation> &locs, std::string msg ) {
        messages_.emplace_back(FileMessage{false, locs, std::move(msg)});
    }

    void FileStatus::warning(const std::string &filename, const Location &loc, std::string msg ) {
        messages_.emplace_back(FileMessage{false, std::vector<FileLocation>{FileLocation{filename,loc}}, std::move(msg)});
    }

    void FileStatus::error(const std::string &filename, const Location &loc, std::string msg ) {
        ++num_errors_;
        messages_.emplace_back(FileMessage{true, std::vector<FileLocation>{FileLocation{filename,loc}}, std::move(msg)});
    }

    bool FileStatus::hasErrors() const {
        return num_errors_ != 0;
    }

    bool FileStatus::hasWarnings() const {
        return messages_.size() > num_errors_;
    }
}

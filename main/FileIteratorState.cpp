//
// Created by ivan on 05.04.2021.
//

#include "FileIteratorState.h"

#include <QDateTime>

typedef int (*FileComparator)(const QFileInfo &f1, const QFileInfo &f2);

class Comparator {
public:
    Comparator(FileComparator cmp, bool reverse) : cmp(cmp), reverse(reverse) {}

    inline bool operator()(const QFileInfo &f1, const QFileInfo &f2) {
        int result = (*cmp)(f1, f2);

        return result != 0 && (reverse ? result > 0 : result < 0);
    }

private:
    FileComparator cmp;
    bool reverse;
};

inline int sortByName(const QFileInfo &f1, const QFileInfo &f2) {
    return f1.fileName().compare(f2.fileName());
}

inline int sortByTime(const QFileInfo &f1, const QFileInfo &f2) {
    int result = f1.lastModified().msecsTo(f2.lastModified());
    return result == 0 ? sortByName(f1, f2) : result;
}

inline Comparator comparator(QDir::SortFlags sortFlags) {
    bool reverse = sortFlags.testFlag(QDir::Reversed);
    FileComparator cmp;
    if (sortFlags.testFlag(QDir::Time)) {
        cmp = sortByTime;
    } else {
        cmp = sortByName;
    }
    return {cmp, reverse};
}

bool FileIteratorState::sameDir(const QDir &dir) const {
    return !files.isEmpty() && dir == files.at(0).dir();
}

bool FileIteratorState::sameFile(const QFileInfo &info) {
    return files.at(uk) == info;
}

void FileIteratorState::locate(const QFileInfo &file) {
    if (file.isReadable()) {
        if (sortBy == QDir::NoSort) {
            for (int i = uk; i < files.size(); i++) {
                if (files.at(i).fileName() == file.fileName()) {
                    uk = i;
                    return;
                }
            }
            for (int i = uk - 1; i >= 0; i--) {
                if (files.at(i).fileName() == file.fileName()) {
                    uk = i;
                    return;
                }
            }
        } else {
            Comparator cmp = comparator(sortBy);
            int l = -1, r = files.size();
            while (l + 1 < r) {
                int mid = (l + r) / 2;
                QFileInfo f(files.at(mid));
                if (cmp(file, f)) {
                        r = mid;
                } else {
                    if (f.fileName() == file.fileName()) {
                        r = mid;
                        break;
                    }
                    l = mid;
                }
            }
            uk = r;
        }
    }
}

QFileInfo FileIteratorState::next() {
    if (uk >= files.size() - 1)
        return QFileInfo();
    return files.at(++uk);
}

QFileInfo FileIteratorState::previous() {
    if (uk == 0)
        return QFileInfo();
    return files.at(--uk);
}

void FileIteratorState::sort() {
    QFileInfo f(current());
    if (sortBy != QDir::NoSort) {
        std::sort(files.begin(), files.end(), comparator(sortBy));
    }
    locate(f);
}

QFileInfo FileIteratorState::current() const {
    return uk < files.size() ? files.at(uk) : QFileInfo();
}

void FileIteratorState::remove() {
    if (uk < files.size()) {
        files.removeAt(uk--);
        if (uk < 0) {
            uk = 0;
        }
    }
}

void FileIteratorState::goBack() {
    if (!files.isEmpty()) {
        uk = static_cast<int>(files.size()) - 1;
    }
}

void FileIteratorState::goFront() {
    uk = 0;
}

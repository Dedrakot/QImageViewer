#include "IteratorState.h"
#include <QDir>
#include <QDateTime>

typedef bool (*FileComparator)(const QFileInfo &f1, const QFileInfo &f2);

class Comparator {
public:
    Comparator(FileComparator cmp, bool reverse) : cmp(cmp), reverse(reverse) {}

    inline bool operator()(const QFileInfo &f1, const QFileInfo &f2) {
        bool result = (*cmp)(f1, f2);
        return reverse == !result;
    }

private:
    FileComparator cmp;
    bool reverse;
};

bool sortByName(const QFileInfo &f1, const QFileInfo &f2) {
    int r = f1.fileName().compare(f2.fileName());
    return r < 0;
}

bool sortByTime(const QFileInfo &f1, const QFileInfo &f2) {
    qint64 r = f1.lastModified().msecsTo(f2.lastModified());
    return r > 0;
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

bool IteratorState::sameDir(const QDir &dir) const {
    return !files.isEmpty() && dir == files.at(0).dir();
}

bool IteratorState::sameFile(const QFileInfo &info) {
    return files.at(uk) == info;
}

void IteratorState::locate(const QFileInfo &file) {
    if (file.isReadable()) {
        if (sortBy == QDir::NoSort) {
            for (int i = uk; i < files.size(); i++) {
                if (files.at(i) == file) {
                    uk = i;
                    return;
                }
            }
            for (int i = uk - 1; i >= 0; i--) {
                if (files.at(i) == file) {
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
                    if (f == file) {
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

bool IteratorState::canIterate() const {
    return files.size() > 1;
}

QFileInfo IteratorState::next() {
    ++uk;
    if (uk >= files.size()) {
        uk = 0;
    }
    return files.at(uk);
}

QFileInfo IteratorState::previous() {
    --uk;
    if (uk < 0) {
        uk = files.isEmpty() ? 0 : files.size() - 1;
    }
    return files.at(uk);
}

void IteratorState::sort() {
    QFileInfo f(current());
    if (sortBy != QDir::NoSort) {
        std::sort(files.begin(), files.end(), comparator(sortBy));
    }
    locate(f);
}

QFileInfo IteratorState::current() const {
    return uk < files.size() ? files.at(uk) : QFileInfo();
}

void IteratorState::remove() {
    if (uk < files.size()) {
        files.removeAt(uk--);
        if (uk < 0) {
            uk = 0;
        }
    }
}

QString IteratorState::filePath() const {
    return uk < files.size() ? files.at(uk).filePath() : QString();
}



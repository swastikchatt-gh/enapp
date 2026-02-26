#include <QApplication>
#include <QMainWindow>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QFileSystemModel>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QMessageBox>
#include <QInputDialog>
#include <QClipboard>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QCloseEvent>
#include <QKeySequence>
#include <QDebug>
#include <QDesktopServices>

class FileManagerWindow : public QMainWindow
{
    Q_OBJECT

public:
    FileManagerWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setWindowTitle("MyFileManager");
        setWindowIcon(QIcon(":/app-icon")); // embedded icon

        // Central splitter
        QSplitter *splitter = new QSplitter(this);
        setCentralWidget(splitter);

        // File system model
        model = new QFileSystemModel(this);
        model->setRootPath(QDir::homePath());
        model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files);

        // Tree view (sidebar) – shows directory tree
        treeView = new QTreeView(splitter);
        treeView->setModel(model);
        treeView->setRootIndex(model->index(QDir::homePath()));
        treeView->hideColumn(1); // size
        treeView->hideColumn(2); // type
        treeView->hideColumn(3); // date modified
        treeView->setHeaderHidden(true);
        treeView->setSelectionMode(QAbstractItemView::SingleSelection);
        treeView->setMaximumWidth(250);

        // List view (main area) – shows contents of current directory
        listView = new QListView(splitter);
        listView->setModel(model);
        listView->setRootIndex(model->index(QDir::homePath()));
        listView->setViewMode(QListView::IconMode);
        listView->setIconSize(QSize(48, 48));
        listView->setGridSize(QSize(80, 80));
        listView->setResizeMode(QListView::Adjust);
        listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        listView->setWordWrap(true);
        listView->setWrapping(true);

        splitter->addWidget(treeView);
        splitter->addWidget(listView);
        splitter->setStretchFactor(1, 1); // list view gets more space

        // Create actions
        createActions();
        createMenus();
        createToolBars();
        createStatusBar();

        // Connect signals
        connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, &FileManagerWindow::onTreeViewCurrentChanged);
        connect(listView, &QListView::doubleClicked,
                this, &FileManagerWindow::onItemDoubleClicked);

        // Initialize location line edit in toolbar
        locationEdit = new QLineEdit(this);
        locationEdit->setText(QDir::homePath());
        connect(locationEdit, &QLineEdit::returnPressed, this, &FileManagerWindow::onLocationEdited);

        // Add location edit to toolbar
        locationAction = toolBar->addWidget(locationEdit);
        locationAction->setText(tr("Location"));

        // Set initial focus
        listView->setFocus();

        // Update status bar
        updateStatusBar();
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        event->accept();
    }

private slots:
    void onTreeViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
    {
        Q_UNUSED(previous);
        if (current.isValid()) {
            QString path = model->filePath(current);
            listView->setRootIndex(model->index(path));
            locationEdit->setText(path);
            updateStatusBar();
        }
    }

    void onItemDoubleClicked(const QModelIndex &index)
    {
        if (!index.isValid()) return;
        QString path = model->filePath(index);
        QFileInfo info(path);
        if (info.isDir()) {
            // Navigate into directory
            listView->setRootIndex(model->index(path));
            locationEdit->setText(path);
            // Also update tree selection
            QModelIndex treeIndex = model->index(path);
            treeView->setCurrentIndex(treeIndex);
            treeView->expand(treeIndex);
        } else {
            // Try to open file with default application
            openFile(path);
        }
    }

    void onLocationEdited()
    {
        QString newPath = locationEdit->text();
        QDir dir(newPath);
        if (dir.exists()) {
            QModelIndex index = model->index(newPath);
            listView->setRootIndex(index);
            treeView->setCurrentIndex(index);
            treeView->expand(index);
            locationEdit->setText(newPath);
        } else {
            QMessageBox::warning(this, tr("Invalid path"),
                                 tr("The path '%1' does not exist.").arg(newPath));
            locationEdit->setText(model->filePath(listView->rootIndex()));
        }
        updateStatusBar();
    }

    void goHome()
    {
        QString home = QDir::homePath();
        QModelIndex index = model->index(home);
        listView->setRootIndex(index);
        treeView->setCurrentIndex(index);
        treeView->expand(index);
        locationEdit->setText(home);
        updateStatusBar();
    }

    void goUp()
    {
        QModelIndex current = listView->rootIndex();
        if (current.isValid()) {
            QModelIndex parent = current.parent();
            if (parent.isValid()) {
                listView->setRootIndex(parent);
                treeView->setCurrentIndex(parent);
                treeView->expand(parent);
                locationEdit->setText(model->filePath(parent));
            }
        }
        updateStatusBar();
    }

    void goBack()
    {
        // For simplicity, we don't implement full history; just up.
        // A real file manager would have a stack.
        goUp();
    }

    void goForward()
    {
        // Not implemented in simple version
    }

    void refresh()
    {
        model->setRootPath(model->rootPath()); // force refresh?
        listView->update();
        treeView->update();
    }

    void newFolder()
    {
        QModelIndex current = listView->rootIndex();
        if (!current.isValid()) return;

        QString basePath = model->filePath(current);
        bool ok;
        QString folderName = QInputDialog::getText(this, tr("New Folder"),
                                                    tr("Folder name:"),
                                                    QLineEdit::Normal,
                                                    tr("New Folder"), &ok);
        if (ok && !folderName.isEmpty()) {
            QString newPath = basePath + QDir::separator() + folderName;
            QDir dir(basePath);
            if (dir.mkdir(folderName)) {
                // model will automatically update
            } else {
                QMessageBox::warning(this, tr("Error"),
                                     tr("Failed to create folder."));
            }
        }
    }

    void deleteItems()
    {
        QModelIndexList selected = listView->selectionModel()->selectedIndexes();
        if (selected.isEmpty()) return;

        QStringList paths;
        for (const QModelIndex &index : selected) {
            paths << model->filePath(index);
        }

        QString message = tr("Are you sure you want to delete %1 item(s)?")
                             .arg(selected.count());
        if (QMessageBox::question(this, tr("Confirm Delete"), message,
                                   QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            for (const QString &path : paths) {
                QFileInfo info(path);
                if (info.isDir()) {
                    QDir dir(path);
                    if (!dir.removeRecursively()) {
                        QMessageBox::warning(this, tr("Error"),
                                             tr("Failed to delete folder '%1'.").arg(path));
                    }
                } else {
                    if (!QFile::remove(path)) {
                        QMessageBox::warning(this, tr("Error"),
                                             tr("Failed to delete file '%1'.").arg(path));
                    }
                }
            }
        }
    }

    void renameItem()
    {
        QModelIndexList selected = listView->selectionModel()->selectedIndexes();
        if (selected.size() != 1) {
            QMessageBox::information(this, tr("Rename"),
                                     tr("Please select exactly one item to rename."));
            return;
        }

        QModelIndex index = selected.first();
        QString oldPath = model->filePath(index);
        QFileInfo info(oldPath);
        QString oldName = info.fileName();

        bool ok;
        QString newName = QInputDialog::getText(this, tr("Rename"),
                                                tr("New name:"),
                                                QLineEdit::Normal,
                                                oldName, &ok);
        if (ok && !newName.isEmpty() && newName != oldName) {
            QString newPath = info.path() + QDir::separator() + newName;
            if (QFile::rename(oldPath, newPath)) {
                // model updates automatically
            } else {
                QMessageBox::warning(this, tr("Error"),
                                     tr("Failed to rename."));
            }
        }
    }

    void copyItems()
    {
        QModelIndexList selected = listView->selectionModel()->selectedIndexes();
        if (selected.isEmpty()) return;

        QList<QUrl> urls;
        for (const QModelIndex &index : selected) {
            urls << QUrl::fromLocalFile(model->filePath(index));
        }

        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(urls);

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setMimeData(mimeData);
        cutMode = false; // copy
    }

    void cutItems()
    {
        QModelIndexList selected = listView->selectionModel()->selectedIndexes();
        if (selected.isEmpty()) return;

        QList<QUrl> urls;
        for (const QModelIndex &index : selected) {
            urls << QUrl::fromLocalFile(model->filePath(index));
        }

        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(urls);

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setMimeData(mimeData);
        cutMode = true; // cut
    }

    void pasteItems()
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if (!mimeData->hasUrls()) return;

        QList<QUrl> urls = mimeData->urls();
        QString destPath = model->filePath(listView->rootIndex());

        for (const QUrl &url : urls) {
            QString srcPath = url.toLocalFile();
            if (srcPath.isEmpty()) continue;

            QFileInfo info(srcPath);
            QString destFile = destPath + QDir::separator() + info.fileName();

            if (cutMode) {
                // Move
                if (!QFile::rename(srcPath, destFile)) {
                    // If rename fails (cross-device), try copy+delete
                    if (info.isDir()) {
                        // recursive copy is complex; we'll just show error
                        QMessageBox::warning(this, tr("Error"),
                                             tr("Failed to move folder. Cross-device moves not supported."));
                    } else {
                        if (QFile::copy(srcPath, destFile)) {
                            QFile::remove(srcPath);
                        } else {
                            QMessageBox::warning(this, tr("Error"),
                                                 tr("Failed to move file."));
                        }
                    }
                }
            } else {
                // Copy
                if (info.isDir()) {
                    // Recursive copy not implemented; just show error
                    QMessageBox::warning(this, tr("Error"),
                                         tr("Folder copy not implemented."));
                } else {
                    if (!QFile::copy(srcPath, destFile)) {
                        QMessageBox::warning(this, tr("Error"),
                                             tr("Failed to copy file."));
                    }
                }
            }
        }
        cutMode = false; // reset after paste
    }

    void about()
    {
        QMessageBox::about(this, tr("About MyFileManager"),
                           tr("MyFileManager – A simple Qt6 file manager.\n\n"
                              "Icon from icons/image.png."));
    }

    void updateStatusBar()
    {
        QString path = model->filePath(listView->rootIndex());
        int itemCount = model->rowCount(listView->rootIndex());
        statusLabel->setText(tr("%1 — %2 items").arg(path).arg(itemCount));
    }

private:
    void createActions()
    {
        // File operations
        homeAct = new QAction(QIcon::fromTheme("go-home"), tr("&Home"), this);
        homeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
        connect(homeAct, &QAction::triggered, this, &FileManagerWindow::goHome);

        upAct = new QAction(QIcon::fromTheme("go-up"), tr("&Up"), this);
        upAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Up));
        connect(upAct, &QAction::triggered, this, &FileManagerWindow::goUp);

        backAct = new QAction(QIcon::fromTheme("go-previous"), tr("&Back"), this);
        backAct->setShortcut(QKeySequence::Back);
        connect(backAct, &QAction::triggered, this, &FileManagerWindow::goBack);

        forwardAct = new QAction(QIcon::fromTheme("go-next"), tr("&Forward"), this);
        forwardAct->setShortcut(QKeySequence::Forward);
        connect(forwardAct, &QAction::triggered, this, &FileManagerWindow::goForward);

        refreshAct = new QAction(QIcon::fromTheme("view-refresh"), tr("&Refresh"), this);
        refreshAct->setShortcut(QKeySequence::Refresh);
        connect(refreshAct, &QAction::triggered, this, &FileManagerWindow::refresh);

        newFolderAct = new QAction(QIcon::fromTheme("folder-new"), tr("&New Folder..."), this);
        newFolderAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
        connect(newFolderAct, &QAction::triggered, this, &FileManagerWindow::newFolder);

        deleteAct = new QAction(QIcon::fromTheme("edit-delete"), tr("&Delete"), this);
        deleteAct->setShortcut(QKeySequence::Delete);
        connect(deleteAct, &QAction::triggered, this, &FileManagerWindow::deleteItems);

        renameAct = new QAction(QIcon::fromTheme("edit-rename"), tr("&Rename..."), this);
        renameAct->setShortcut(QKeySequence(Qt::Key_F2));
        connect(renameAct, &QAction::triggered, this, &FileManagerWindow::renameItem);

        copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
        copyAct->setShortcut(QKeySequence::Copy);
        connect(copyAct, &QAction::triggered, this, &FileManagerWindow::copyItems);

        cutAct = new QAction(QIcon::fromTheme("edit-cut"), tr("&Cut"), this);
        cutAct->setShortcut(QKeySequence::Cut);
        connect(cutAct, &QAction::triggered, this, &FileManagerWindow::cutItems);

        pasteAct = new QAction(QIcon::fromTheme("edit-paste"), tr("&Paste"), this);
        pasteAct->setShortcut(QKeySequence::Paste);
        connect(pasteAct, &QAction::triggered, this, &FileManagerWindow::pasteItems);

        exitAct = new QAction(tr("&Quit"), this);
        exitAct->setShortcut(QKeySequence::Quit);
        connect(exitAct, &QAction::triggered, this, &QWidget::close);

        aboutAct = new QAction(tr("&About"), this);
        connect(aboutAct, &QAction::triggered, this, &FileManagerWindow::about);
    }

    void createMenus()
    {
        QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
        fileMenu->addAction(newFolderAct);
        fileMenu->addSeparator();
        fileMenu->addAction(deleteAct);
        fileMenu->addAction(renameAct);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);

        QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
        editMenu->addAction(copyAct);
        editMenu->addAction(cutAct);
        editMenu->addAction(pasteAct);

        QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
        viewMenu->addAction(homeAct);
        viewMenu->addAction(upAct);
        viewMenu->addAction(backAct);
        viewMenu->addAction(forwardAct);
        viewMenu->addSeparator();
        viewMenu->addAction(refreshAct);

        QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
        helpMenu->addAction(aboutAct);
    }

    void createToolBars()
    {
        toolBar = addToolBar(tr("Main"));
        toolBar->addAction(backAct);
        toolBar->addAction(forwardAct);
        toolBar->addAction(upAct);
        toolBar->addAction(homeAct);
        toolBar->addSeparator();
        toolBar->addAction(refreshAct);
        toolBar->addSeparator();
        toolBar->addAction(newFolderAct);
        toolBar->addAction(deleteAct);
        toolBar->addAction(renameAct);
        toolBar->addSeparator();
        toolBar->addAction(copyAct);
        toolBar->addAction(cutAct);
        toolBar->addAction(pasteAct);
    }

    void createStatusBar()
    {
        statusLabel = new QLabel(this);
        statusBar()->addWidget(statusLabel);
        updateStatusBar();
    }

    void openFile(const QString &path)
    {
        // Try to open with default application using QDesktopServices
        QUrl url = QUrl::fromLocalFile(path);
        if (!QDesktopServices::openUrl(url)) {
            QMessageBox::warning(this, tr("Error"),
                                 tr("Failed to open file."));
        }
    }

    QFileSystemModel *model;
    QTreeView *treeView;
    QListView *listView;
    QToolBar *toolBar;
    QLabel *statusLabel;
    QLineEdit *locationEdit;
    QAction *locationAction;

    QAction *homeAct, *upAct, *backAct, *forwardAct, *refreshAct;
    QAction *newFolderAct, *deleteAct, *renameAct;
    QAction *copyAct, *cutAct, *pasteAct;
    QAction *exitAct, *aboutAct;

    bool cutMode = false; // true if last copy was a cut
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FileManagerWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"

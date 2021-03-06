#include "FileManager.h"
#include <boost/filesystem.hpp>
#include <QtWidgets>
#include "NKTestConstants.h"


void file_manager::init()
{
    for (boost::filesystem::directory_iterator it{ boost::filesystem::absolute(nk_quiz_directory) }; it != boost::filesystem::directory_iterator{}; ++it)
    {
        if (boost::filesystem::is_regular_file(it->path()) && boost::filesystem::extension(it->path()) == nk_extension)
        {
            std::string name = boost::filesystem::basename(it->path());
            active_files.insert(name);
            emit file_added(QString::fromStdString(name));
        }
    }
}

void file_manager::add_file(const std::string& full_path)
{
    if (boost::filesystem::extension(full_path) == ".csv")
    {
        std::string filename = boost::filesystem::basename(full_path);

        if (filename.substr(0, 8) == "tangorin")
        {
            QMessageBox* should_rename = new QMessageBox{ QMessageBox::Question, "NKTest", "Do you want to rename the file?", QMessageBox::Yes | QMessageBox::No, nullptr /*see below, except iono yet what to do with this one. Prolly refactor to the main class*/ };
            if (should_rename->exec() == QMessageBox::Yes)
            {
                bool ok;
                QString new_name = QInputDialog::getText(nullptr /*-||-*/, "NKTest", "New filename:", QLineEdit::Normal, QDir::home().dirName(), &ok);
                if (ok && !new_name.isEmpty()) filename = new_name.toStdString();
            }
        }

        boost::filesystem::path
            source{ full_path },
            destination{ nk_quiz_directory / (filename + nk_extension) };
        boost::filesystem::copy_file(source, destination);

        active_files.insert(filename);
        emit file_added(QString::fromStdString(filename));
    }
    else
    {
        QMessageBox* invalid_file = new QMessageBox{ QMessageBox::Warning, "Warning", "Invalid file", QMessageBox::Ok, nullptr /*can't be like that but Imma wait till I have a dedicated error handler I mean a little memory leak can't hurt, right???*/ };
        invalid_file->exec();
    }
}

void file_manager::remove_file(const std::string& filename)
{
    active_files.erase(filename);
    std::remove(boost::filesystem::path{ nk_quiz_directory / (filename + nk_extension) }.string().c_str());
    emit file_removed(QString::fromStdString(filename));
}

void file_manager::open_dialog()
{
    std::set<std::string> files_to_remove;

    file_manager_dialog dlg{ active_files, files_to_remove };
    if (dlg.exec() == QDialog::Accepted)
    {
        for (const auto& filename : files_to_remove)
        {
            remove_file(filename);
        }
    }
}

file_manager_dialog::file_manager_dialog(std::set<std::string>& active_files, std::set<std::string>& files_to_remove)
{
    QVBoxLayout* layout = new QVBoxLayout;
    for (auto& filename : active_files)
    {
        QWidget* file_bar = new QWidget{ this };
        layout->addWidget(file_bar);
        QHBoxLayout* bar_layout = new QHBoxLayout;
        file_bar->setLayout(bar_layout);

        QLabel* name = new QLabel{ QString::fromStdString(filename), this };
        bar_layout->addWidget(name);

        QPushButton* remove = new QPushButton{ "Remove", this };
        connect(remove, &QPushButton::pressed, [&files_to_remove, name, remove]() {
            if (remove->text() == "Remove")
            {
                name->setStyleSheet("QLabel { color : red; }");
                remove->setText("Undo");

                files_to_remove.insert(name->text().toStdString());
            }
            else
            {
                name->setStyleSheet("QLabel { color : black; }");
                remove->setText("Remove");

                files_to_remove.erase(name->text().toStdString());
            }

        });
        bar_layout->addWidget(remove);
    }

    if (active_files.empty())
    {
        QLabel* empty_msg = new QLabel{ "No files to show", this };
        empty_msg->setAlignment(Qt::AlignCenter);
        layout->addWidget(empty_msg);
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox{ QDialogButtonBox::Ok | QDialogButtonBox::Cancel };
    connect(buttonBox, &QDialogButtonBox::accepted, this, &file_manager_dialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &file_manager_dialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}
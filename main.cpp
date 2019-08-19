/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2018 <tsujan2000@gmail.com>
 *
 * Arqiver is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arqiver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <https://spdx.org/licenses/GPL-3.0+.html>
 */

#include <signal.h>
#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTextStream>
#include <QFileInfo>

#include "mainWin.h"

void handleQuitSignals(const std::vector<int>& quitSignals) {
  auto handler = [](int sig) ->void {
    Q_UNUSED(sig);
    QCoreApplication::quit();
  };

  for(int sig : quitSignals)
    signal(sig, handler);
}

int main(int argc, char **argv) {
  const QString name = "Arqiver";
  const QString version = "0.3.0";
  const QString option = QString::fromUtf8(argv[1]);
  if (option == "--help" || option == "-h") {
    QTextStream out (stdout);
    out << "Arqiver - Simple Qt5 archive manager\n"\
           "          based on libarchive, gzip and 7z\n\n"\
            "Usage:\n	arqiver [option] [ARCHIVE] [FILES]\n\n"\
            "Options:\n\n"\
            "--help or -h     Show this help and exit.\n"\
            "--version or -v  Show version information and exit.\n"\
            "--sx             Extract an archive: arqiver --sx ARCHIVE\n"\
            "--sa             Archive file(s): arqiver --sa FILE(S)\n"\
            "--ax             Auto-extract archive(s): arqiver --ax ARCHIVE(S)\n"\
            "--aa             Auto-archive file(s): arqiver --aa ARCHIVE FILE(S)" << endl;
    return 0;
  }
  else if (option == "--version" || option == "-v") {
    QTextStream out (stdout);
    out << name << " " << version <<  endl;
    return 0;
  }

  QApplication a(argc, argv);
  a.setApplicationName(name);
  a.setApplicationVersion(version);
#ifndef Q_OS_WIN
  handleQuitSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

  a.setAttribute(Qt::AA_UseHighDpiPixmaps);

  QStringList langs(QLocale::system().uiLanguages());
  QString lang;
  if (!langs.isEmpty())
    lang = langs.first().replace('-', '_');

  QTranslator qtTranslator;
  if (!qtTranslator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
    if (!langs.isEmpty()) {
      lang = langs.first().split (QLatin1Char ('-')).first();
      qtTranslator.load ("qt_" + lang, QLibraryInfo::location (QLibraryInfo::TranslationsPath));
    }
  }
  a.installTranslator(&qtTranslator);

  QTranslator ArqTranslator;
  ArqTranslator.load("arqiver_" + lang, DATADIR "/arqiver/translations");
  a.installTranslator (&ArqTranslator);

  QStringList args;
  for (int i = 1; i < argc; i++) {
    if (QString(argv[i]).startsWith("--"))
      args << QString(argv[i]);
    else {
      QString str = QString::fromUtf8 (argv[i]);
      if (str.contains ("/")) {
        if (str.startsWith ("file://"))
          str = QUrl (str).toLocalFile();
        /* always an absolute path (works around KDE's double slash bug too) */
        QFileInfo fInfo (str);
        str = fInfo.absoluteFilePath();
      }
      args << str;
    }
  }

  Arqiver::mainWin W;
  // see mainWin::loadArguments() for an explanation
  if (args.isEmpty())
    W.show();
  else
    W.loadArguments(args);
  return  a.exec();
}

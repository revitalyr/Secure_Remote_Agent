// core/remote_executor.cpp
#include "remote_executor.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QProcessEnvironment>

RemoteExecutor::RemoteExecutor(QObject* parent)
    : QObject(parent)
    , executionTimeout(30000)
    , currentProcess(nullptr)
    , timeoutTimer(new QTimer(this))
{
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &RemoteExecutor::onExecutionTimeout);
}

RemoteExecutor::~RemoteExecutor() {
    if (currentProcess) {
        currentProcess->kill();
        currentProcess->deleteLater();
    }
}

void RemoteExecutor::setCommandWhitelist(const QStringList& commands) {
    commandWhitelist = commands;
}

void RemoteExecutor::setExecutionTimeout(int timeoutMs) {
    executionTimeout = timeoutMs;
}

QVariantMap RemoteExecutor::executeCommand(const QString& command, const QStringList& args) {
    if (!validateCommand(command)) {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Command not allowed: " + command;
        result["exit_code"] = -1;
        return result;
    }

    QProcess process;
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    
    QStringList sanitizedArgs = sanitizeArguments(args);
    process.start(command, sanitizedArgs);
    
    if (!process.waitForStarted()) {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Failed to start process: " + process.errorString();
        result["exit_code"] = -1;
        return result;
    }

    if (!process.waitForFinished(executionTimeout)) {
        process.kill();
        process.waitForFinished();
        
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Command timeout";
        result["exit_code"] = -1;
        result["stdout"] = QString::fromUtf8(process.readAllStandardOutput());
        result["stderr"] = QString::fromUtf8(process.readAllStandardError());
        return result;
    }

    QVariantMap result;
    result["success"] = (process.exitCode() == 0);
    result["exit_code"] = process.exitCode();
    result["stdout"] = QString::fromUtf8(process.readAllStandardOutput());
    result["stderr"] = QString::fromUtf8(process.readAllStandardError());
    return result;
}

void RemoteExecutor::executeCommandAsync(const QString& command, const QStringList& args) {
    if (!validateCommand(command)) {
        emit commandError("Command not allowed: " + command);
        return;
    }

    if (currentProcess) {
        currentProcess->kill();
        currentProcess->deleteLater();
    }

    currentProcess = new QProcess(this);
    currentProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    
    connect(currentProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RemoteExecutor::onProcessFinished);
    connect(currentProcess, &QProcess::errorOccurred,
            this, &RemoteExecutor::onProcessError);
    connect(currentProcess, &QProcess::readyReadStandardOutput,
            this, &RemoteExecutor::onProcessReadyReadStandardOutput);
    connect(currentProcess, &QProcess::readyReadStandardError,
            this, &RemoteExecutor::onProcessReadyReadStandardError);

    QStringList sanitizedArgs = sanitizeArguments(args);
    currentProcess->start(command, sanitizedArgs);
    
    if (!currentProcess->waitForStarted()) {
        emit commandError("Failed to start process: " + currentProcess->errorString());
        currentProcess->deleteLater();
        currentProcess = nullptr;
        return;
    }

    currentOutput.clear();
    currentError.clear();
    timeoutTimer->start(executionTimeout);
}

QVariantMap RemoteExecutor::executeScript(const QString& scriptPath, const QStringList& args) {
    QFileInfo fileInfo(scriptPath);
    if (!fileInfo.exists()) {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Script not found: " + scriptPath;
        result["exit_code"] = -1;
        return result;
    }

    QString extension = fileInfo.suffix().toLower();
    QString command;
    
    if (extension == "ps1") {
        command = "powershell";
        QStringList psArgs;
        psArgs << "-ExecutionPolicy" << "Bypass" << "-File" << scriptPath;
        psArgs << args;
        return executeCommand(command, psArgs);
    } else if (extension == "bat" || extension == "cmd") {
        command = "cmd";
        QStringList cmdArgs;
        cmdArgs << "/c" << scriptPath;
        cmdArgs << args;
        return executeCommand(command, cmdArgs);
    } else if (extension == "sh") {
        command = "bash";
        QStringList shArgs;
        shArgs << scriptPath;
        shArgs << args;
        return executeCommand(command, shArgs);
    } else {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Unsupported script type: " + extension;
        result["exit_code"] = -1;
        return result;
    }
}

QVariantMap RemoteExecutor::listDirectory(const QString& directoryPath) {
    QDir dir(directoryPath);
    if (!dir.exists()) {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Directory not found: " + directoryPath;
        return result;
    }

    QVariantMap result;
    result["success"] = true;
    result["path"] = directoryPath;
    
    QVariantList files;
    QVariantList directories;
    
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo& entry : entries) {
        QVariantMap entryInfo;
        entryInfo["name"] = entry.fileName();
        entryInfo["size"] = entry.size();
        entryInfo["is_file"] = entry.isFile();
        entryInfo["is_dir"] = entry.isDir();
        entryInfo["last_modified"] = entry.lastModified().toString(Qt::ISODate);
        
        if (entry.isFile()) {
            files.append(entryInfo);
        } else {
            directories.append(entryInfo);
        }
    }
    
    result["files"] = files;
    result["directories"] = directories;
    return result;
}

QVariantMap RemoteExecutor::getFileInfo(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QVariantMap result;
        result["success"] = false;
        result["error"] = "File not found: " + filePath;
        return result;
    }

    QVariantMap result;
    result["success"] = true;
    result["name"] = fileInfo.fileName();
    result["path"] = fileInfo.filePath();
    result["size"] = fileInfo.size();
    result["is_file"] = fileInfo.isFile();
    result["is_dir"] = fileInfo.isDir();
    result["is_sym_link"] = fileInfo.isSymLink();
    result["last_modified"] = fileInfo.lastModified().toString(Qt::ISODate);
    result["created"] = fileInfo.birthTime().toString(Qt::ISODate);
    result["permissions"] = static_cast<int>(fileInfo.permissions());
    return result;
}

bool RemoteExecutor::deleteFile(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return false;
    }

    if (fileInfo.isDir()) {
        return QDir(filePath).removeRecursively();
    } else {
        return QFile(filePath).remove();
    }
}

bool RemoteExecutor::createDirectory(const QString& directoryPath) {
    QDir dir;
    return dir.mkpath(directoryPath);
}

bool RemoteExecutor::killProcess(qint64 pid) {
    QProcess process;
    QStringList args;
    args << "/PID" << QString::number(pid) << "/F";
    
    process.start("taskkill", args);
    return process.waitForFinished() && process.exitCode() == 0;
}

QVariantMap RemoteExecutor::listProcesses() {
    QProcess process;
    QStringList args;
    args << "tasklist" << "/FO" << "CSV" << "/NH";
    
    process.start("cmd", QStringList() << "/c" << args.join(" "));
    
    if (!process.waitForFinished(10000)) {
        process.kill();
        process.waitForFinished();
        
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Command timeout";
        return result;
    }

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    QVariantList processes;
    for (const QString& line : lines) {
        QStringList parts = line.split(',');
        if (parts.size() >= 2) {
            QVariantMap procInfo;
            procInfo["name"] = parts[0].remove('"');
            procInfo["pid"] = parts[1].remove('"').toInt();
            if (parts.size() >= 5) {
                procInfo["memory"] = parts[4].remove('"').trimmed();
            }
            processes.append(procInfo);
        }
    }

    QVariantMap result;
    result["success"] = true;
    result["processes"] = processes;
    return result;
}

void RemoteExecutor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    timeoutTimer->stop();
    
    QVariantMap result;
    result["success"] = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    result["exit_code"] = exitCode;
    result["stdout"] = currentOutput;
    result["stderr"] = currentError;
    
    emit commandCompleted(result);
    
    currentProcess->deleteLater();
    currentProcess = nullptr;
}

void RemoteExecutor::onProcessError(QProcess::ProcessError error) {
    timeoutTimer->stop();
    
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = "Process failed to start";
            break;
        case QProcess::Crashed:
            errorString = "Process crashed";
            break;
        case QProcess::Timedout:
            errorString = "Process timed out";
            break;
        case QProcess::ReadError:
            errorString = "Read error";
            break;
        case QProcess::WriteError:
            errorString = "Write error";
            break;
        default:
            errorString = "Unknown process error";
            break;
    }
    
    emit commandError(errorString);
    
    if (currentProcess) {
        currentProcess->deleteLater();
        currentProcess = nullptr;
    }
}

void RemoteExecutor::onProcessReadyReadStandardOutput() {
    if (currentProcess) {
        QString output = QString::fromUtf8(currentProcess->readAllStandardOutput());
        currentOutput += output;
        emit commandOutput(output);
    }
}

void RemoteExecutor::onProcessReadyReadStandardError() {
    if (currentProcess) {
        QString error = QString::fromUtf8(currentProcess->readAllStandardError());
        currentError += error;
        emit commandError(error);
    }
}

void RemoteExecutor::onExecutionTimeout() {
    if (currentProcess) {
        currentProcess->kill();
        currentProcess->waitForFinished();
        
        QVariantMap result;
        result["success"] = false;
        result["error"] = "Command timeout";
        result["exit_code"] = -1;
        result["stdout"] = currentOutput;
        result["stderr"] = currentError;
        
        emit commandCompleted(result);
        
        currentProcess->deleteLater();
        currentProcess = nullptr;
    }
}

bool RemoteExecutor::validateCommand(const QString& command) {
    if (commandWhitelist.isEmpty()) {
        return true;
    }
    
    QString commandBase = QFileInfo(command).baseName();
    return commandWhitelist.contains(commandBase) || commandWhitelist.contains(command);
}

QStringList RemoteExecutor::sanitizeArguments(const QStringList& args) {
    QStringList sanitized;
    for (const QString& arg : args) {
        QString sanitizedArg = arg;
        sanitizedArg.remove('\n');
        sanitizedArg.remove('\r');
        sanitizedArg.remove('\0');
        sanitized.append(sanitizedArg);
    }
    return sanitized;
}

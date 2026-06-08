#include "voice.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace alphabet {

VoiceInput::VoiceInput() {}

VoiceInput::~VoiceInput() {
    stop();
}

bool VoiceInput::start(const std::string& python_path) {
    if (running_)
        return true;

    std::string script_path = "tools/voice_server.py";

#ifdef _WIN32
    char bin_path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(nullptr, bin_path, MAX_PATH);
    if (len > 0) {
        std::string bin_dir(bin_path);
        auto pos = bin_dir.rfind('\\');
        if (pos != std::string::npos) {
            bin_dir = bin_dir.substr(0, pos);
            std::string candidate = bin_dir + "\\..\\tools\\voice_server.py";
            DWORD attr = GetFileAttributesA(candidate.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES) {
                script_path = candidate;
            }
        }
    }

    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE hChildStdinRead, hChildStdinWrite;
    HANDLE hChildStdoutRead, hChildStdoutWrite;

    if (!CreatePipe(&hChildStdinRead, &hChildStdinWrite, &sa, 0))
        return false;
    if (!CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &sa, 0)) {
        CloseHandle(hChildStdinRead);
        CloseHandle(hChildStdinWrite);
        return false;
    }

    SetHandleInformation(hChildStdinWrite, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {sizeof(STARTUPINFOA)};
    si.hStdInput = hChildStdinRead;
    si.hStdOutput = hChildStdoutWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    std::string cmd_line = python_path + " " + script_path;
    char cmd_buf[4096];
    strncpy(cmd_buf, cmd_line.c_str(), sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';

    PROCESS_INFORMATION pi = {};
    BOOL ok = CreateProcessA(nullptr, cmd_buf, nullptr, nullptr, TRUE, 0,
                             nullptr, nullptr, &si, &pi);

    CloseHandle(hChildStdinRead);
    CloseHandle(hChildStdoutWrite);

    if (!ok) {
        CloseHandle(hChildStdinWrite);
        CloseHandle(hChildStdoutRead);
        return false;
    }

    h_stdin_ = hChildStdinWrite;
    h_stdout_ = hChildStdoutRead;
    h_process_ = pi.hProcess;
    CloseHandle(pi.hThread);

    running_ = true;
    std::string response = read_response();
    return !response.empty();
#else
    char bin_path[4096] = {};
    ssize_t len = readlink("/proc/self/exe", bin_path, sizeof(bin_path) - 1);
    if (len > 0) {
        bin_path[len] = '\0';
        std::string bin_dir(bin_path);
        auto pos = bin_dir.rfind('/');
        if (pos != std::string::npos) {
            bin_dir = bin_dir.substr(0, pos);
            std::string candidate = bin_dir + "/../tools/voice_server.py";
            if (access(candidate.c_str(), F_OK) == 0) {
                script_path = candidate;
            }
        }
    }

    int child_stdin[2];
    int child_stdout[2];

    if (pipe(child_stdin) < 0 || pipe(child_stdout) < 0) {
        return false;
    }

    pid_ = fork();
    if (pid_ < 0) {
        close(child_stdin[0]);
        close(child_stdin[1]);
        close(child_stdout[0]);
        close(child_stdout[1]);
        return false;
    }

    if (pid_ == 0) {
        close(child_stdin[1]);
        close(child_stdout[0]);

        dup2(child_stdin[0], STDIN_FILENO);
        dup2(child_stdout[1], STDOUT_FILENO);

        close(child_stdin[0]);
        close(child_stdout[1]);

        execlp(python_path.c_str(), python_path.c_str(), script_path.c_str(), nullptr);
        _exit(127);
    }

    close(child_stdin[0]);
    close(child_stdout[1]);

    stdin_fd_ = child_stdin[1];
    stdout_fd_ = child_stdout[0];

    fcntl(stdout_fd_, F_SETFL, O_NONBLOCK);

    running_ = true;
    std::string response = read_response();
    return !response.empty();
#endif
}

bool VoiceInput::send_command(const std::string& json_cmd) {
    if (!running_)
        return false;

    std::string cmd = json_cmd + "\n";

#ifdef _WIN32
    DWORD written = 0;
    BOOL ok = WriteFile(h_stdin_, cmd.c_str(), static_cast<DWORD>(cmd.size()), &written, nullptr);
    return ok && written == cmd.size();
#else
    ssize_t written = write(stdin_fd_, cmd.c_str(), cmd.size());
    return written == (ssize_t)cmd.size();
#endif
}

std::string VoiceInput::read_response() {
    if (!running_)
        return "";

    char buf[4096];
    std::string result;
    int attempts = 0;
    int max_attempts = 300;

#ifdef _WIN32
    while (attempts < max_attempts) {
        DWORD available = 0;
        if (PeekNamedPipe(h_stdout_, nullptr, 0, nullptr, &available, nullptr) && available > 0) {
            DWORD n = 0;
            if (ReadFile(h_stdout_, buf, sizeof(buf) - 1, &n, nullptr) && n > 0) {
                buf[n] = '\0';
                result += buf;
                if (result.find('\n') != std::string::npos) break;
            } else {
                break;
            }
        } else {
            Sleep(100);
            attempts++;
        }
    }
#else
    while (attempts < max_attempts) {
        ssize_t n = read(stdout_fd_, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            result += buf;
            if (result.find('\n') != std::string::npos) break;
        } else if (n == 0) {
            break;
        } else {
            usleep(100000);
            attempts++;
        }
    }
#endif

    auto pos = result.find('\n');
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }

    return result;
}

bool VoiceInput::init_language(const std::string& lang) {
    current_lang_ = lang;
    std::string cmd = "{\"cmd\": \"init\", \"lang\": \"" + lang + "\"}";
    if (!send_command(cmd))
        return false;

    std::string response = read_response();
    return response.find("\"ready\"") != std::string::npos;
}

std::string VoiceInput::listen(int timeout) {
    std::string cmd = "{\"cmd\": \"listen\", \"timeout\": " + std::to_string(timeout) + "}";
    if (!send_command(cmd))
        return "";

    std::string response = read_response();

    auto text_pos = response.find("\"text\"");
    if (text_pos == std::string::npos)
        return "";

    auto colon_pos = response.find(":", text_pos);
    if (colon_pos == std::string::npos)
        return "";

    auto quote_start = response.find("\"", colon_pos + 1);
    if (quote_start == std::string::npos)
        return "";

    auto quote_end = response.find("\"", quote_start + 1);
    if (quote_end == std::string::npos)
        return "";

    return response.substr(quote_start + 1, quote_end - quote_start - 1);
}

bool VoiceInput::is_available() const {
    return running_;
}

void VoiceInput::stop() {
    if (!running_)
        return;

    send_command("{\"cmd\": \"quit\"}");

#ifdef _WIN32
    Sleep(500);
    if (h_stdin_ != INVALID_HANDLE_VALUE) {
        CloseHandle(h_stdin_);
        h_stdin_ = INVALID_HANDLE_VALUE;
    }
    if (h_stdout_ != INVALID_HANDLE_VALUE) {
        CloseHandle(h_stdout_);
        h_stdout_ = INVALID_HANDLE_VALUE;
    }
    if (h_process_ != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(h_process_, 2000);
        TerminateProcess(h_process_, 0);
        CloseHandle(h_process_);
        h_process_ = INVALID_HANDLE_VALUE;
    }
#else
    usleep(500000);
    if (stdin_fd_ >= 0) {
        close(stdin_fd_);
        stdin_fd_ = -1;
    }
    if (stdout_fd_ >= 0) {
        close(stdout_fd_);
        stdout_fd_ = -1;
    }
    if (pid_ > 0) {
        kill(pid_, SIGTERM);
        waitpid(pid_, nullptr, 0);
        pid_ = -1;
    }
#endif

    running_ = false;
}

std::string VoiceInput::get_status() {
    if (!running_)
        return "Voice server not running";

    send_command("{\"cmd\": \"status\"}");
    return read_response();
}

} // namespace alphabet

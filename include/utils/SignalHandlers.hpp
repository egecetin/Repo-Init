#pragma once

#include <csignal>
#include <functional>
#include <map>
#include <vector>

namespace utils
{
    /**
     * @brief A class that provides utility functions for handling signals.
     */
    class SignalHandlers {
    private:
        static std::map<int, std::function<void(int)>> signalHandlers;

        /**
         * @brief Internal signal handler function that invokes the registered signal handler for a given signal.
         * @param[in] signal The signal number.
         */
        static void signalHandler(int signal)
        {
            if (signalHandlers.find(signal) != signalHandlers.end())
            {
                signalHandlers[signal](signal);
            }
        }

    public:
        SignalHandlers() = delete;

        /**
         * @brief Sets a signal handler for a specific signal.
         * @param[in] signal The signal number.
         * @param[in] handler The signal handler function.
         */
        static void setSignalHandler(int signal, std::function<void(int)> handler)
        {
            signalHandlers[signal] = handler;
            std::signal(signal, SignalHandlers::signalHandler);
        }

        /**
         * @brief Removes the signal handler for a specific signal.
         * @param[in] signal The signal number.
         */
        static void removeSignalHandler(int signal)
        {
            signalHandlers.erase(signal);
            std::signal(signal, SIG_DFL);
        }

        /**
         * @brief Removes all registered signal handlers.
         */
        static void removeSignalHandlers()
        {
            for (auto& handler : signalHandlers)
            {
                std::signal(handler.first, SIG_DFL);
            }
            signalHandlers.clear();
        }

        /**
         * @brief Ignores a specific signal.
         * @param[in] signal The signal number.
         */
        static void ignoreSignal(int signal) { std::signal(signal, SIG_IGN); }

        /**
         * @brief Ignores multiple signals.
         * @param[in] signals A vector of signal numbers.
         */
        static void ignoreSignals(const std::vector<int>& signals)
        {
            for (auto& signal : signals)
            {
                std::signal(signal, SIG_IGN);
            }
        }

        /**
         * @brief Restores the default behavior for a specific signal.
         * @param[in] signal The signal number.
         */
        static void restoreSignal(int signal) { std::signal(signal, SIG_DFL); }

        /**
         * @brief Restores the default behavior for multiple signals.
         * @param[in] signals A vector of signal numbers.
         */
        static void restoreSignals(const std::vector<int>& signals)
        {
            for (auto& signal : signals)
            {
                std::signal(signal, SIG_DFL);
            }
        }

        /**
         * @brief Restores the default behavior for all signals.
         */
        static void restoreAllSignals()
        {
            for (int signal = 1; signal < 32; signal++)
            {
                std::signal(signal, SIG_DFL);
            }
        }

        /**
         * @brief Blocks a specific signal.
         * @param[in] signal The signal number.
         */
        static void blockSignal(int signal)
        {
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, signal);
            sigprocmask(SIG_BLOCK, &mask, NULL);
        }

        /**
         * @brief Blocks multiple signals.
         * @param[in] signals A vector of signal numbers.
         */
        static void blockSignals(const std::vector<int>& signals)
        {
            sigset_t mask;
            sigemptyset(&mask);
            for (auto& signal : signals)
            {
                sigaddset(&mask, signal);
            }
            sigprocmask(SIG_BLOCK, &mask, NULL);
        }

        /**
         * @brief Unblocks a specific signal.
         * @param[in] signal The signal number.
         */
        static void unblockSignal(int signal)
        {
            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, signal);
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
        }

        /**
         * @brief Unblocks multiple signals.
         * @param[in] signals A vector of signal numbers.
         */
        static void unblockSignals(const std::vector<int>& signals)
        {
            sigset_t mask;
            sigemptyset(&mask);
            for (auto& signal : signals)
            {
                sigaddset(&mask, signal);
            }
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
        }

        /**
         * @brief Unblocks all signals.
         */
        static void unblockAllSignals()
        {
            sigset_t mask;
            sigemptyset(&mask);
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
        }

        /**
         * @brief Blocks all signals.
         */
        static void blockAllSignals()
        {
            sigset_t mask;
            sigfillset(&mask);
            sigprocmask(SIG_BLOCK, &mask, NULL);
        }

        /**
         * @brief Blocks all signals except the specified signals.
         * @param[in] signals A vector of signal numbers.
         */
        static void blockAllSignalsExcept(const std::vector<int>& signals)
        {
            sigset_t mask;
            sigfillset(&mask);
            for (auto& signal : signals)
            {
                sigdelset(&mask, signal);
            }
            sigprocmask(SIG_BLOCK, &mask, NULL);
        }
    };
} // namespace utils

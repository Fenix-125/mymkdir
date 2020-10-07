// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

// Remember to include ALL the necessary headers!
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>

#define DIR_PERMISSIONS     (0755)
#define MKDIR_FAIL_CODE     (-1)


static inline bool create_parents(const std::string &path);

static inline bool dir_exists(const std::string &path);

int main(int argc, char **argv) {
    std::vector<std::string> dirs_to_create{};
    bool create_parents_flag;

    namespace po = boost::program_options;

    po::options_description visible("Supported options");
    visible.add_options()
            ("help,h", "Print this help message.")
            ("parents,p", po::value<bool>(&create_parents_flag)->zero_tokens(),
             "No error if existing, make parent directories as needed");

    po::options_description hidden("Hidden options");
    hidden.add_options()
            ("dir", po::value<std::vector<std::string>>(&dirs_to_create)->composing(),
             "directory name to be created.");

    po::positional_options_description p;
    p.add("dir", -1);

    po::options_description all("All options");
    all.add(visible).add(hidden);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(all).positional(p).run(), vm);
    } catch (const boost::wrapexcept<boost::program_options::unknown_option> &e) {
        std::cerr << "Error: unknown option '" << e.get_option_name() << "'" << std::endl;
        return EXIT_FAILURE;
    }

    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage:\n  mymkdir [dir]\n" << visible << std::endl;
        return EXIT_SUCCESS;
    }
    if (dirs_to_create.empty()) {
        std::cerr << "Error: no directory names specified!" << std::endl;
        return EXIT_FAILURE;
    }

    char *tmp_dir_path;
    for (const auto &path : dirs_to_create) {
        tmp_dir_path = strdup(path.data());
        if (tmp_dir_path == nullptr) {
            std::cerr << "Error: unexpected string copy error" << std::endl;
            return EXIT_FAILURE;
        }
        if (dir_exists(dirname(tmp_dir_path))) {
        } else if (create_parents_flag) {
            if (!create_parents(path)) {
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Error: path not found" << std::endl;
            return EXIT_FAILURE;
        }
        if (mkdir(path.data(), DIR_PERMISSIONS) == MKDIR_FAIL_CODE) {
            std::cerr << "Error: failed to create directory" << std::endl;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}


static inline bool dir_exists(const std::string &path) {
    struct stat status{};
    if (stat(path.data(), &status) == 0) {
        return static_cast<bool>(S_ISDIR(status.st_mode));
    }
    return false;
}

static inline bool create_parents(const std::string &path) {
    size_t offset = 0u;
    std::string parent_path{};
    parent_path.reserve(path.size());
    while ((offset = path.find('/', offset)) != std::string::npos) {
        parent_path = path.substr(0, offset++);
        if (!dir_exists(parent_path)) {
            if (mkdir(parent_path.data(), DIR_PERMISSIONS) == MKDIR_FAIL_CODE) {
                std::cerr << "Error: can not create path parent directories!" << std::endl;
                return false;
            }
        }
    }
    return true;
}

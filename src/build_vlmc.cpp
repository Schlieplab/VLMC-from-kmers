#include "vlmc_from_kmers/build_vlmc.hpp"
#include "vlmc_from_kmers/bic.hpp"
#include "vlmc_from_kmers/dvstar.hpp"

int main(int argc, char *argv[]) {
  CLI::App app{"Variable-length Markov chain construction construction using "
               "k-mer counter."};

  vlmc::cli_arguments arguments{};
  add_options(app, arguments);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  bool tmp_path_existed_before = std::filesystem::exists(arguments.tmp_path);

  if (arguments.in_or_out_of_core == vlmc::Core::out) {
      std::filesystem::create_directories(arguments.tmp_path);
  }
  vlmc::configure_stxxl(arguments.tmp_path);

  if (arguments.mode == vlmc::Mode::build) {
    if (arguments.out_path.empty() || arguments.fasta_path.empty()) {
        std::cerr << "Error: Both a --fasta-path and an --out-path need to be given." << std::endl;
        return EXIT_FAILURE;
    }
    int exit_code = vlmc::build_vlmc(
        arguments.fasta_path, arguments.max_depth, arguments.min_count,
        arguments.threshold, arguments.out_path, arguments.tmp_path,
        arguments.in_or_out_of_core, arguments.pseudo_count_amount);

    if (!tmp_path_existed_before) {
      std::filesystem::remove_all(arguments.tmp_path);
    }

    return exit_code;

  } else if (arguments.mode == vlmc::Mode::build_from_kmc_db) {
    int exit_code = vlmc::build_vlmc_from_kmc_db(
        arguments.in_path, arguments.max_depth, arguments.min_count,
        arguments.threshold, arguments.out_path, arguments.in_or_out_of_core,
        arguments.pseudo_count_amount);

    return exit_code;
  } else if (arguments.mode == vlmc::Mode::dump) {
    return vlmc::dump_path(arguments.in_path, arguments.out_path);
  } else if (arguments.mode == vlmc::Mode::score_sequence) {
    vlmc::negative_log_likelihood(
        arguments.fasta_path, arguments.tmp_path, arguments.in_path,
        arguments.in_or_out_of_core, arguments.max_depth);
  } else if (arguments.mode == vlmc::Mode::bic) {

    vlmc::find_best_parameters_bic(
        arguments.fasta_path, arguments.max_depth, arguments.min_count,
        arguments.out_path, arguments.tmp_path, arguments.in_or_out_of_core);
  } else if (arguments.mode == vlmc::Mode::dvstar_dissimliarity) {
    double dissimilarity =
        vlmc::dvstar(arguments.in_path, arguments.to_path, 0);
    std::cout << dissimilarity << std::endl;
  }

  //if (!tmp_path_existed_before) {
  //  std::filesystem::remove_all(arguments.tmp_path);
  //}

  return EXIT_SUCCESS;
}

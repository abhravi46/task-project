#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "framework/framework.hpp"
#include "interface/interface.hpp"

void print_usage(const char *program_name) {
  std::cerr << "Usage: " << program_name << " [options]\n"
            << "Options:\n"
            << "  -t <type>      Interface type (loopback is the only valid "
               "value for now)\n"
            << "  -p <port>      Port number\n"
            << "  -a <address>   IP address (not used if loopback)\n"
            << "  -o <file>      Output file location where network data is "
               "written to\n"
            << "  -i <file>      Input file to transmit\n"
            << "  -d <file>      Debug file where debug output is written to\n"
            << "  -s             Run as server\n"
            << "  -c             Run as client\n"
            << "  -h             Show this help message\n";
}

int main(int argc, char *argv[]) {
  // Default values
  std::string interface_type;
  int port = 0;
  std::string address;
  std::string output_file;
  std::string input_file;
  std::string debug_file;
  bool is_server = false;
  bool is_client = false;

  // Parse command line options
  int opt;
  while ((opt = getopt(argc, argv, "t:p:a:o:i:d:sch")) != -1) {
    switch (opt) {
    case 't':
      interface_type = optarg;
      break;
    case 'p':
      port = std::stoi(optarg);
      break;
    case 'a':
      address = optarg;
      break;
    case 'o':
      output_file = optarg;
      break;
    case 'i':
      input_file = optarg;
      break;
    case 'd':
      debug_file = optarg;
      break;
    case 's':
      is_server = true;
      break;
    case 'c':
      is_client = true;
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }

  // Validate required parameters
  if (interface_type.empty()) {
    std::cerr << "Error: Interface type (-t) is required\n";
    print_usage(argv[0]);
    return 1;
  }

  if (interface_type != "loopback") {
    std::cerr << "Error: Only 'loopback' interface type is supported for now\n";
    return 1;
  }

  if (is_server && is_client) {
    std::cerr << "Error: Cannot specify both -s (server) and -c (client)\n";
    return 1;
  }

  if (!is_server && !is_client) {
    std::cerr << "Error: Must specify either -s (server) or -c (client)\n";
    print_usage(argv[0]);
    return 1;
  }

  // Setup debug output
  std::ofstream debug_stream;
  std::ostream *debug_out = &std::cerr;
  if (!debug_file.empty()) {
    debug_stream.open(debug_file);
    if (!debug_stream.is_open()) {
      std::cerr << "Error: Cannot open debug file: " << debug_file << "\n";
      return 1;
    }
    debug_out = &debug_stream;
  }

  *debug_out << "Starting " << (is_server ? "server" : "client") << " with "
             << interface_type << " interface\n";
  if (port > 0) {
    *debug_out << "Port: " << port << "\n";
  }
  if (!address.empty()) {
    *debug_out << "Address: " << address << " (not used for loopback)\n";
  }

  // Create the interface
  Interface *interface = nullptr;
  if (interface_type == "loopback") {
    interface = new LocalLoopback(port);
  }

  if (interface == nullptr) {
    std::cerr << "Error: Failed to create interface\n";
    return 1;
  }

  *debug_out << "Interface created successfully\n";

  // Create server or client based on mode
  FrmwrkBase *app = nullptr;
  if (is_server) {
    app = new Server(interface);
    *debug_out << "Created Server instance\n";
  } else {
    app = new Client(interface);
    *debug_out << "Created Client instance\n";
  }

  // Open the connection
  if (!app->Open()) {
    std::cerr << "Error: Failed to open " << (is_server ? "server" : "client")
              << "\n";
    delete app;
    delete interface;
    return 1;
  }

  *debug_out << (is_server ? "Server" : "Client") << " opened successfully\n";

  // For server: listen for connections
  // For client: connect to server
  if (is_server) {
    *debug_out << "Server listening for connections...\n";
    if (!app->Listen()) {
      std::cerr << "Error: Server failed to listen\n";
      app->Close();
      delete app;
      delete interface;
      return 1;
    }
    *debug_out << "Server listening and client connected\n";
  } else {
    *debug_out << "Client connecting to server...\n";
    if (!app->Connect()) {
      std::cerr << "Error: Client failed to connect\n";
      app->Close();
      delete app;
      delete interface;
      return 1;
    }
    *debug_out << "Client connected successfully\n";
  }

  // Handle output file (receive data)
  std::ofstream output_stream;
  if (!output_file.empty()) {
    output_stream.open(output_file, std::ios::binary);
    if (!output_stream.is_open()) {
      std::cerr << "Error: Cannot open output file: " << output_file << "\n";
    } else {
      *debug_out << "Output will be written to: " << output_file << "\n";
    }
  }

  auto transmit = [&]() {
    if (!input_file.empty()) {
      *debug_out << "Reading input from: " << input_file << "\n";
      std::ifstream input_stream(input_file, std::ios::binary);
      if (!input_stream.is_open()) {
        std::cerr << "Error: Cannot open input file: " << input_file << "\n";
      } else {
        input_stream.seekg(0, std::ios::end);
        size_t file_size = input_stream.tellg();
        input_stream.seekg(0, std::ios::beg);

        char *buffer = new char[file_size];
        input_stream.read(buffer, file_size);
        input_stream.close();

        ssize_t bytes_written = app->Write(buffer, file_size);
        *debug_out << "Wrote " << bytes_written << " bytes from input file\n";

        delete[] buffer;
      }
    }
  };

  auto receive = [&]() {
    const size_t kBufferSize = 4096;
    char read_buffer[kBufferSize];
    ssize_t bytes_read = app->Read(read_buffer, kBufferSize);

    if (bytes_read > 0) {
      *debug_out << "Read " << bytes_read << " bytes\n";

      if (output_stream.is_open()) {
        output_stream.write(read_buffer, bytes_read);
        *debug_out << "Wrote " << bytes_read << " bytes to output file\n";
      } else {
        std::cout.write(read_buffer, bytes_read);
      }
    } else if (bytes_read == 0) {
      *debug_out << "No data available to read\n";
    } else {
      *debug_out << "Error reading data\n";
    }
  };

  if (is_server) {
    receive();
    transmit();
  } else {
    transmit();
    receive();
  }

  // Cleanup
  if (output_stream.is_open()) {
    output_stream.close();
  }

  // Disconnect (for client) or just close
  if (is_client) {
    app->Disconnect();
    *debug_out << "Client disconnected\n";
  }

  app->Close();
  *debug_out << (is_server ? "Server" : "Client") << " closed\n";

  delete app;
  delete interface;

  *debug_out << "Application completed successfully\n";

  if (debug_stream.is_open()) {
    debug_stream.close();
  }

  return 0;
}

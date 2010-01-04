#ifndef NEXTGEN_NETWORK
#define NEXTGEN_NETWORK

#include "common.h"

namespace nextgen
{
    namespace network
    {
        class stream
        {
            public: typedef asio::streambuf streambuf_type;

            public: streambuf_type& get_buffer() const
            {
                auto self = *this;

                return self->streambuf;
            }

            public: struct variables
            {
                variables()
                {
                    NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                }

                ~variables()
                {
                    NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                }

                streambuf_type streambuf;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(stream, variables);
        };

        struct basic_service_variables
        {
            typedef asio::io_service service_type;

            basic_service_variables()
            {
                NEXTGEN_DEBUG_CONSTRUCTOR(*this);
            }

            ~basic_service_variables()
            {
                NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
            }

            service_type service;
        };

        template<typename VariablesType = basic_service_variables>
        class basic_service
        {
            public: typedef VariablesType variables_type;
            private: typedef asio::io_service service_type;

            public: void update()
            {
                auto self = *this;

                self->service.poll();
                self->service.reset();
            }

            public: service_type& get_service()
            {
                auto self = *this;

                return self->service;
            }

            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_service, variables_type);
        };

        namespace ip
        {
            namespace network
            {
                struct layer_base_variables
                {
                    typedef std::string host_type;
                    typedef uint32_t port_type;

                    layer_base_variables(host_type const& host = null, port_type port = null) : host(host), port(port)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    ~layer_base_variables()
                    {
                        NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                    }

                    host_type host;
                    port_type port;
                };

                template<typename VariablesType = layer_base_variables>
                class layer_base
                {
                    public: typedef VariablesType variables_type;

                    public: typedef std::string host_type;
                    public: typedef uint32_t port_type;

                    public: host_type const& get_host()
                    {
                        auto self = *this;

                        return self->host;
                    }

                    public: void set_host(host_type const& host)
                    {
                        auto self = *this;

                        self->host = host;
                    }

                    public: port_type get_port()
                    {
                        auto self = *this;

                        return self->port;
                    }

                    public: void set_port(port_type port)
                    {
                        auto self = *this;

                        self->port = port;
                    }

                    NEXTGEN_ATTACH_SHARED_VARIABLES(layer_base, variables_type);
                };

                namespace ipv4
                {
                    struct basic_layer_variables : public layer_base_variables
                    {
                        typedef layer_base_variables base_type;

                        basic_layer_variables(host_type const& host = null, port_type port = null) : base_type(host, port)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                    };

                    template<typename VariablesType = basic_layer_variables>
                    class basic_layer : public layer_base<VariablesType>
                    {
                        public: typedef VariablesType variables_type;
                        public: typedef layer_base<variables_type> base_type;

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }

                namespace ipv6
                {
                    struct basic_layer_variables : public layer_base_variables
                    {
                        typedef layer_base_variables base_type;

                        basic_layer_variables(host_type const& host = null, port_type port = null) : base_type(host, port)
                        {

                        }
                    };

                    template<typename VariablesType = basic_layer_variables>
                    class basic_layer : public layer_base<VariablesType>
                    {
                        public: typedef VariablesType variables_type;
                        public: typedef layer_base<variables_type> base_type;

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }
            }
        }

        class address
        {
            union v4
            {
                int value;

                struct
                {
                    uint8_t first;
                    uint8_t second;
                    uint8_t third;
                    uint8_t forth;
                };
            };

            public: bool is_valid()
            {
                auto self = *this;

                return self->valid;
            }

            public: struct variables
            {
                variables(uint32_t address) : valid(true)
                {
                    NEXTGEN_DEBUG_CONSTRUCTOR(*this);

                    this->value.value = address;
                }

                variables(std::string const& address) : valid(true)
                {
                    NEXTGEN_DEBUG_CONSTRUCTOR(*this);

                    this->from_string(address);
                }

                ~variables()
                {
                    NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                }

                void from_string(std::string const& s)
                {
                    boost::regex_error paren(boost::regex_constants::error_paren);

                    try
                    {
                        boost::match_results<std::string::const_iterator> what;
                        boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                        std::string::const_iterator start = s.begin();
                        std::string::const_iterator end = s.end();

                        if(boost::regex_search(start, end, what, boost::regex("^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$"), flags))
                        {
                            auto first = boost::lexical_cast<uint32_t>(what[1]);
                            auto second = boost::lexical_cast<uint32_t>(what[2]);
                            auto third = boost::lexical_cast<uint32_t>(what[3]);
                            auto forth = boost::lexical_cast<uint32_t>(what[4]);

                            if(!(first >= 0 && first <= 255
                            && second >= 0 && second <= 255
                            && third >= 0 && third <= 255
                            && forth >= 0 && forth <= 255))
                            {
                                this->valid = false;

                                return;
                            }

                            this->value.first = boost::numeric_cast<uint8_t>(first);
                            this->value.second = boost::numeric_cast<uint8_t>(second);
                            this->value.third = boost::numeric_cast<uint8_t>(third);
                            this->value.forth = boost::numeric_cast<uint8_t>(forth);
                        }
                        else
                        {
                            this->valid = false;

                            return;
                        }
                    }
                    catch(boost::regex_error const& e)
                    {
                        std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;

                        this->valid = false;

                        return;
                    }
                }

                v4 value;
                bool valid;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(address, variables);
        };

        class address_range
        {
            public: bool is_within_range(address a)
            {
                auto self = *this;

                if(a->value.first <= self->upper->value.first && a->value.first >= self->lower->value.first
                && a->value.second <= self->upper->value.second && a->value.second >= self->lower->value.second
                && a->value.third <= self->upper->value.third && a->value.third >= self->lower->value.third
                && a->value.forth <= self->upper->value.forth && a->value.forth >= self->lower->value.forth)
                {
                    std::cout << boost::numeric_cast<uint32_t>(self->lower->value.first) << "." << boost::numeric_cast<uint32_t>(self->lower->value.second) << "." << boost::numeric_cast<uint32_t>(self->lower->value.third) << "." << boost::numeric_cast<uint32_t>(self->lower->value.forth) << std::endl;
                    std::cout << boost::numeric_cast<uint32_t>(a->value.first) << "." << boost::numeric_cast<uint32_t>(a->value.second) << "." << boost::numeric_cast<uint32_t>(a->value.third) << "." << boost::numeric_cast<uint32_t>(a->value.forth) << std::endl;
                    std::cout << boost::numeric_cast<uint32_t>(self->upper->value.first) << "." << boost::numeric_cast<uint32_t>(self->upper->value.second) << "." << boost::numeric_cast<uint32_t>(self->upper->value.third) << "." << boost::numeric_cast<uint32_t>(self->upper->value.forth) << std::endl;
                    return true;
                }

                return false;
            }

            public: struct variables
            {
                variables(address&& lower, address&& upper) : lower(lower), upper(upper)
                {
                    NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                }

                variables(std::string const& lower, std::string const& upper) : lower(lower), upper(upper)
                {
                    NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                }

                ~variables()
                {
                    NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                }

                address lower;
                address upper;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(address_range, variables);
        };

        typedef ip::network::ipv4::basic_layer<> ipv4_address;
        typedef ip::network::ipv6::basic_layer<> ipv6_address;

        namespace ip
        {
            namespace transport
            {
                template<typename NetworkLayerType>
                struct layer_base_variables
                {
                    typedef NetworkLayerType network_layer_type;

                    typedef basic_service<> service_type;
                    typedef std::string host_type;
                    typedef uint32_t port_type;
                    typedef uint32_t timeout_type;
                    typedef asio::deadline_timer timer_type;

                    typedef std::function<void()> base_event_type;
                    typedef base_event_type connect_successful_event_type;
                    typedef base_event_type connect_failure_event_type;
                    typedef base_event_type receive_successful_event_type;
                    typedef base_event_type receive_failure_event_type;
                    typedef base_event_type send_successful_event_type;
                    typedef base_event_type send_failure_event_type;
                    typedef base_event_type quit_successful_event_type;
                    typedef base_event_type quit_failure_event_type;
                    typedef base_event_type accept_failure_event_type;
                    typedef base_event_type close_event_type;

                    layer_base_variables(service_type service) : service(service), timer(service.get_service()), timeout(180)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    ~layer_base_variables()
                    {
                        NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                    }

                    event<send_successful_event_type> send_successful_event;
                    event<send_failure_event_type> send_failure_event;
                    event<connect_successful_event_type> connect_successful_event;
                    event<connect_failure_event_type> connect_failure_event;
                    event<receive_successful_event_type> receive_successful_event;
                    event<receive_failure_event_type> receive_failure_event;
                    event<accept_failure_event_type> accept_failure_event;
                    event<close_event_type> close_event;

                    service_type service;
                    timer_type timer;
                    network_layer_type network_layer;
                    timeout_type timeout;
                };

                template<typename NetworkLayerType, typename VariablesType = layer_base_variables<NetworkLayerType>>
                class layer_base
                {
                    public: typedef VariablesType variables_type;

                    NEXTGEN_ATTACH_SHARED_VARIABLES(layer_base, variables_type);
                };

                struct accepter_base_variables
                {
                    typedef uint32_t port_type;

                    accepter_base_variables(port_type port = null) : port(port)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    ~accepter_base_variables()
                    {
                        NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                    }

                    port_type port;
                };

                template<typename VariablesType = accepter_base_variables>
                class accepter_base
                {
                    public: typedef VariablesType variables_type;

                    NEXTGEN_ATTACH_SHARED_VARIABLES(accepter_base, variables_type);
                };

                namespace tcp
                {
                    struct basic_accepter_variables : public accepter_base_variables
                    {
                        typedef accepter_base_variables base_type;
                        typedef asio::ip::tcp::acceptor accepter_type;
                        typedef basic_service<> service_type;

                        basic_accepter_variables(service_type service) : base_type(), accepter_(service.get_service())
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_accepter_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                        accepter_type accepter_;
                    };

                    template<typename VariablesType = basic_accepter_variables>
                    class basic_accepter : public accepter_base<VariablesType>
                    {
                        public: typedef VariablesType variables_type;
                        public: typedef accepter_base<variables_type> base_type;

                        private: typedef asio::ip::tcp::acceptor accepter_type;
                        private: typedef asio::ip::tcp::socket socket_type;
                        private: typedef asio::ip::tcp::endpoint endpoint_type;
                        public: typedef basic_service<> service_type;
                        public: typedef uint32_t port_type;

                        public: void open(port_type port)
                        {
                            auto self = *this;

                            endpoint_type endpoint(asio::ip::tcp::v4(), port);

                            try
                            {
                                self->accepter_.open(endpoint.protocol());
                                self->accepter_.set_option(asio::ip::tcp::acceptor::reuse_address(true));

                                self->accepter_.bind(endpoint);
                                self->accepter_.listen();

                                // successfully binded port
                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "[nextgen:network:ip:transport:tcp:accepter] Successfully binded port " << port << "." << std::endl;

                                self->port = port;
                            }
                            catch(std::exception& e)
                            {
                                std::cout << "[nextgen:network:ip:transport:tcp:accepter] Failed to bind port " << port << "." << std::endl;
                            }
                        }

                        public: bool is_open()
                        {
                            auto self = *this;

                            return self->accepter_.is_open();
                        }

                        public: port_type get_port() const
                        {
                            auto self = *this;

                            return self->port;
                        }

                        public: template<typename handler_type> void accept(socket_type& socket, handler_type handler)
                        {
                            auto self = *this;

                            self->accepter_.async_accept(socket, handler);
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_accepter, base_type);
                    };

                    template<typename LayerType, typename NetworkLayerType>
                    struct basic_layer_variables : public layer_base_variables<NetworkLayerType>
                    {
                        typedef LayerType layer_type;
                        typedef NetworkLayerType network_layer_type;
                        typedef layer_base_variables<network_layer_type> base_type;
                        typedef basic_layer_variables<layer_type, network_layer_type> this_type;

                        typedef std::function<void(layer_type)> accept_successful_event_type;
                        typedef asio::ip::tcp::socket socket_type;
                        typedef asio::ip::tcp::resolver resolver_type;
                        typedef basic_accepter<> accepter_type;
                        typedef std::function<void(asio::error_code const&)> cancel_handler_type;
                        typedef basic_service<> service_type;
                        typedef asio::ssl::stream<asio::ip::tcp::socket&> ssl_socket_type;

                        basic_layer_variables(service_type service) : base_type(service), service(service), accepter(service), socket(service.get_service()), resolver(service.get_service()), ssl(false), ssl_context(service.get_service(), asio::ssl::context::sslv23_client), ssl_socket(socket, ssl_context)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                        event<accept_successful_event_type> accept_successful_event;

                        service_type service;
                        accepter_type accepter;
                        socket_type socket;
                        resolver_type resolver;
                        cancel_handler_type cancel_handler;
                        bool ssl;
                        asio::ssl::context ssl_context;
                        ssl_socket_type ssl_socket;

                    };

                    template<typename NetworkLayerType>
                    class basic_layer : public layer_base<NetworkLayerType, basic_layer_variables<basic_layer<NetworkLayerType>, NetworkLayerType>>
                    {
                        public: typedef NetworkLayerType network_layer_type;
                        public: typedef basic_layer<network_layer_type> this_type;
                        public: typedef basic_layer_variables<this_type, network_layer_type> variables_type;
                        public: typedef layer_base<network_layer_type, variables_type> base_type;

                        public: typedef basic_service<> service_type;
                        public: typedef std::string host_type;
                        public: typedef uint32_t port_type;
                        public: typedef uint32_t timeout_type;

                        private: typedef asio::ip::tcp::socket socket_type;
                        private: typedef asio::ip::tcp::resolver resolver_type;
                        private: typedef asio::deadline_timer timer_type;
                        public: typedef basic_accepter<> accepter_type;

                        public: typedef std::function<void(asio::error_code const&)> cancel_handler_type;

                        public: typedef std::function<void()> base_event_type;
                        public: typedef base_event_type connect_successful_event_type;
                        public: typedef base_event_type connect_failure_event_type;
                        public: typedef base_event_type receive_successful_event_type;
                        public: typedef base_event_type receive_failure_event_type;
                        public: typedef base_event_type send_successful_event_type;
                        public: typedef base_event_type send_failure_event_type;
                        public: typedef base_event_type quit_successful_event_type;
                        public: typedef base_event_type quit_failure_event_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef base_event_type accept_failure_event_type;
                        public: typedef base_event_type close_event_type;

                        public: typedef asio::ssl::stream<asio::ip::tcp::socket&> ssl_socket_type;

                        public: void initialize() const
                        {
                            auto self = *this;

                            auto& socket = self->socket;

                            self->cancel_handler = [=, &socket](asio::error_code const& error)
                            {
                                if(error == asio::error::operation_aborted)
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:cancel_handler] Timer cancelled" << std::endl;
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:cancel_handler] Timer called back. Closing socket" << std::endl;

                                    // bugfix(daemn): read timer doesn't actually cancel

                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "<socket::cancel> Cancelling socket" << std::endl;

                                    if(socket.native() != asio::detail::invalid_socket)
                                        socket.cancel();
                                    //else
                                    //    std::cout << "<ClientSocket> Guarded an invalid socket." << std::endl;
                                }
                            };
                        }

                        public: host_type const& get_host() const
                        {
                            auto self = *this;

                            return self->network_layer.get_host();
                        }

                        public: void set_host(host_type const& host) const
                        {
                            auto self = *this;

                            self->network_layer.set_host(host);
                        }

                        public: port_type get_port() const
                        {
                            auto self = *this;

                            return self->network_layer.get_port();
                        }

                        public: void set_port(port_type port) const
                        {
                            auto self = *this;

                            self->network_layer.set_port(port);
                        }

                        public: bool is_connected() const
                        {
                            auto self = *this;

                            return self->socket.is_open();
                        }

                        public: void cancel() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::cancel> Cancelling socket (" << self->network_layer.get_host() << ":" << self->network_layer.get_port() << ")" << std::endl;

                            if(self->socket.native() != asio::detail::invalid_socket)
                                self->socket.cancel();
                            //else
                            //    std::cout << "<ClientSocket> Guarded an invalid socket." << std::endl;
                        }

                        public: void close() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::close> Closing socket normally. (" << self->network_layer.get_host() << ":" << self->network_layer.get_port() << ")" << std::endl;

                            if(self->socket.native() != asio::detail::invalid_socket)
                                self->socket.close();

                            self->close_event();
                        }

                        public: size_t bytes_readable() const
                        {
                            auto self = *this;

                            asio::socket_base::bytes_readable command(true);
                            self->socket.io_control(command);

                            return command.get();
                        }

                        public: void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self.set_host(host_);
                            self.set_port(port_);

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::connect> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                            resolver_type::query query(host_, port_ == 80 ? "http" : to_string(port_));

                            if(self->timeout > 0)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::connect> create timer (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                self->timer.expires_from_now(boost::posix_time::seconds(30));
                                self->timer.async_wait(self->cancel_handler);
                            }

                            self->resolver.async_resolve(query,
                            [=](asio::error_code const& error, resolver_type::iterator endpoint_iterator)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::connect handler> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                if(self->timeout > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "<socket::connect handler> resolve success (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                    //todo(daemn) add additional endpoint connection tries
                                    asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

                                    //++endpoint_iterator;

                                    if(NEXTGEN_DEBUG_1)
                                        std::cout << "<socket::connect handler> create timer (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                    if(self->timeout > 0)
                                    {
                                        self->timer.expires_from_now(boost::posix_time::seconds(self->timeout));
                                        self->timer.async_wait(self->cancel_handler);
                                    }

                                    self->socket.async_connect(endpoint, [=](asio::error_code const& error)
                                    {
                                        if(NEXTGEN_DEBUG_1)
                                            std::cout << "<socket::connect handler> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                        if(self->timeout > 0)
                                            self.cancel_timer();

                                        if(!error)
                                        {
                                            successful_handler();
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "<socket::connect handler> Error: " << error.message() << std::endl;

                                            self.close();

                                            failure_handler();
                                        }
                                    });
                                }
                                else
                                {
                                    if(self->timeout > 0)
                                        self.cancel_timer();

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::connect handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            });
                        }

                        public: void cancel_timer() const
                        {
                            auto self = *this;

                            self->timer.cancel();
                        }

                        public: template<typename stream_type> void send(stream_type stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->send_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->send_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::write> create timer (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                            if(self->timeout > 0)
                            {
                                self->timer.expires_from_now(boost::posix_time::seconds(self->timeout));
                                self->timer.async_wait(self->cancel_handler);
                            }

                            auto on_write = [=](asio::error_code const& error, size_t& total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(self->timeout > 0)
                                    self.cancel_timer();

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::write handler> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::write handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            };

                            if(self->ssl)
                                asio::async_write(self->ssl_socket, stream.get_buffer(), on_write);
                            else
                                asio::async_write(self->socket, stream.get_buffer(), on_write);
                        }

                        public: template<typename stream_type> void receive_until(std::string const& delimiter, stream_type stream, receive_successful_event_type successful_handler2 = 0, receive_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2; // bugfix(daemn) weird lambda stack bug, would only accept PBR

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::receive> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                            auto on_read = [=](asio::error_code const& error, uint32_t total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::receive handler> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                if(self->timeout > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::receive handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            };

                            if(self->timeout > 0)
                            {
                                self->timer.expires_from_now(boost::posix_time::seconds(self->timeout));
                                self->timer.async_wait(self->cancel_handler);
                            }

                            if(self->ssl)
                                asio::async_read_until(self->ssl_socket, stream.get_buffer(), delimiter, on_read);
                            else
                                asio::async_read_until(self->socket, stream.get_buffer(), delimiter, on_read);
                        }

                        public: template<typename delimiter_type, typename stream_type> void receive(delimiter_type delimiter, stream_type stream, receive_successful_event_type successful_handler2 = 0, receive_failure_event_type failure_handler2 = 0) const
                        {
                            auto self2 = *this;
                            auto self = self2; // bugfix(daemn) weird lambda stack bug, would only accept PBR

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "<socket::receive> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                            auto on_read = [=](asio::error_code const& error, uint32_t total)
                            {
                                stream.get_buffer(); // bugfix(daemn)

                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "<socket::receive handler> (" << self.get_host() << ":" << self.get_port() << ")" << std::endl;

                                if(self->timeout > 0)
                                    self.cancel_timer();

                                if(!error)
                                {
                                    successful_handler();
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "<socket::receive handler> Error: " << error.message() << std::endl;

                                    self.close();

                                    failure_handler();
                                }
                            };

                            if(self->timeout > 0)
                            {
                                self->timer.expires_from_now(boost::posix_time::seconds(self->timeout));
                                self->timer.async_wait(self->cancel_handler);
                            }

                            if(self->ssl)
                                asio::async_read(self->ssl_socket, stream.get_buffer(), delimiter, on_read);
                            else
                                asio::async_read(self->socket, stream.get_buffer(), delimiter, on_read);
                        }

                        public: void accept(port_type port, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->accept_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->accept_failure_event;

                            if(NEXTGEN_DEBUG_1)
                                std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] " << std::endl;

                            this_type client(self.get_service());

                            if(self->accepter->port != port)
                                self->accepter.open(port);

                            auto on_accept = [=](asio::error_code const& error)
                            {
                                if(NEXTGEN_DEBUG_1)
                                    std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] Trying to accept client..." << std::endl;

                                if(!error)
                                {
                                    client->network_layer.set_host(client->socket.local_endpoint().address().to_string());
                                    client->network_layer.set_port(client->socket.local_endpoint().port());

                                    successful_handler(client);

                                    self.accept(port, successful_handler, failure_handler);
                                }
                                else
                                {
                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << "[nextgen:network:ip:transport:tcp:socket:accept] Error: " << error.message() << std::endl;

                                    failure_handler();
                                }
                            };

                            self->accepter.accept(client->socket, on_accept);
                        }

                        public: socket_type& get_socket() const
                        {
                            auto self = *this;

                            return self->socket;
                        }

                        public: service_type get_service() const
                        {
                            auto self = *this;

                            return self->service;
                        }

                        public: timer_type& get_timer() const
                        {
                            auto self = *this;

                            return self->timer;
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type,
                        {
                            this->initialize();
                        });
                    };
                }
            }
        }

        typedef ip::transport::tcp::basic_layer<ipv4_address> tcp_socket;

        namespace ip
        {
            namespace application
            {
                struct message_base_variables
                {
                    typedef byte_array stream_type;

                    message_base_variables() : state(0)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    ~message_base_variables()
                    {
                        NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                    }

                    std::string content;
                    byte_array stream;
                    uint32_t state;
                };

                template<typename VariablesType>
                class message_base
                {
                    public: typedef VariablesType variables_type;

                    public: typedef byte_array stream_type;

                    public: byte_array get_stream() const
                    {
                        auto self = *this;

                        return self->stream;
                    }

                    public: void pack() const
                    {
                        auto self = *this;

                        std::ostream data_stream(&self->stream.get_buffer());

                        data_stream << self->content;
                    }

                    public: void unpack() const
                    {
                        auto self = *this;

                        std::istream data_stream(&self->stream.get_buffer());

                        self->content = std::string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());
                    }

                    NEXTGEN_ATTACH_SHARED_VARIABLES(message_base, variables_type);
                };

                template<typename LayerType, typename TransportLayerType, typename MessageType>
                struct layer_base_variables
                {
                    typedef MessageType message_type;
                    typedef TransportLayerType transport_layer_type;
                    typedef LayerType layer_type;

                    typedef basic_service<> service_type;

                    typedef std::function<void(message_type)> receive_successful_event_type;
                    typedef std::function<void(layer_type)> accept_successful_event_type;

                    layer_base_variables(service_type service) : transport_layer(service), keep_alive_threshold(0), host(null_str), port(null)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    layer_base_variables(transport_layer_type transport_layer) : transport_layer(transport_layer), keep_alive_threshold(0), host(null_str), port(null)
                    {
                        NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                    }

                    ~layer_base_variables()
                    {
                        NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                    }

                    event<std::function<void()>> send_successful_event;
                    event<std::function<void()>> send_failure_event;
                    event<receive_successful_event_type> receive_successful_event;
                    event<std::function<void()>> receive_failure_event;
                    event<std::function<void()>> connect_successful_event;
                    event<std::function<void()>> connect_failure_event;
                    event<accept_successful_event_type> accept_successful_event;
                    event<std::function<void()>> accept_failure_event;
                    event<std::function<void()>> disconnect_event;

                    timer keep_alive_timer;
                    transport_layer_type transport_layer;
                    uint32_t keep_alive_threshold;
                    ipv4_address proxy_address;
                    std::string host;
                    uint32_t port;
                    uint32_t id;
                };

                template<typename LayerType, typename TransportLayerType, typename MessageType, typename VariablesType = layer_base_variables<LayerType, TransportLayerType, MessageType>>
                class layer_base
                {
                    public: typedef LayerType layer_type;
                    public: typedef VariablesType variables_type;
                    public: typedef MessageType message_type;
                    public: typedef TransportLayerType transport_layer_type;
                    public: typedef layer_base<transport_layer_type, message_type, variables_type> this_type;

                    public: typedef std::function<void()> base_event_type;
                    public: typedef base_event_type connect_successful_event_type;
                    public: typedef base_event_type connect_failure_event_type;
                    public: typedef std::function<void(message_type)> receive_successful_event_type;
                    public: typedef base_event_type receive_failure_event_type;
                    public: typedef base_event_type send_successful_event_type;
                    public: typedef base_event_type send_failure_event_type;
                    public: typedef std::function<void(layer_type)> accept_successful_event_type;
                    public: typedef base_event_type accept_failure_event_type;
                    public: typedef base_event_type disconnect_event_type;

                    public: typedef basic_service<> service_type;
                    public: typedef std::string host_type;
                    public: typedef uint32_t port_type;
                    public: typedef uint32_t timeout_type;
                    public: typedef float keep_alive_threshold_type;

                    public: void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler2 = null, connect_failure_event_type failure_handler2 = null) const
                    {
                        layer_type self = *this;

                        auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                        auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                        if(successful_handler == null)
                            successful_handler = self->connect_successful_event;

                        if(failure_handler == null)
                            failure_handler = self->connect_failure_event;

                        self->host = host_;
                        self->port = port_;

                        self->transport_layer.connect(host_, port_, successful_handler, failure_handler);
                    }

                    public: void reconnect(connect_successful_event_type successful_handler = 0, connect_failure_event_type failure_handler = 0) const
                    {
                        layer_type self = *this;

                        if(NEXTGEN_DEBUG_5)
                            std::cout << "[nextgen::network::layer_base] reconnecting" << std::endl;

                        self.disconnect();
                        self.connect(self->host, self->port, successful_handler, failure_handler);
                    }

                    public: void disconnect() const
                    {
                        layer_type self = *this;

                        self->transport_layer.close();
                    }

                    public: void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                    {
                        layer_type self = *this;

                        if(successful_handler == 0)
                            successful_handler = self->receive_successful_event;

                        if(failure_handler == 0)
                            failure_handler = self->receive_failure_event;

                        message_type response;

                        // todo(daemn) change to receive until EOF
                        self->transport_layer.receive(asio::transfer_at_least(1), response->stream,
                        [=]
                        {
                            response.unpack();

                            successful_handler(response);
                        },
                        failure_handler);
                    }

                    public: template<typename stream_type> void send_stream(stream_type stream, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                    {
                        layer_type self = *this;

                        if(successful_handler == 0)
                            successful_handler = self->send_successful_event;

                        if(failure_handler == 0)
                            failure_handler = self->send_failure_event;

                        self->transport_layer.send(stream, successful_handler, failure_handler);
                    }

                    public: void send(message_type request, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                    {
                        layer_type self = *this;

                        request.pack();

                        self.send_stream(request->stream, successful_handler, failure_handler);
                    }

                    public: void send_and_receive(message_type request, receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                    {
                        layer_type self = *this;

                        if(successful_handler == 0)
                            successful_handler = self->receive_successful_event;

                        if(failure_handler == 0)
                            failure_handler = self->receive_failure_event;

                        self.send(request,
                        [=]
                        {
                            //request_->stream.get_buffer(); // bugfix(daemn)

                            self.receive(successful_handler, failure_handler);
                        },
                        failure_handler);
                    }

                    public: void accept(port_type port, accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
                    {
                        layer_type self = *this;

                        auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                        auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                        if(successful_handler == 0)
                            successful_handler = self->accept_successful_event;

                        if(failure_handler == 0)
                            failure_handler = self->accept_failure_event;

                        self->transport_layer.accept(port,
                        [=](transport_layer_type client)
                        {
                            successful_handler(layer_type(client));
                        },
                        failure_handler);
                    }

                    public: bool is_alive() const
                    {
                        layer_type self = *this;

                        return (self->keep_alive_threshold == 0) ? true : (self->keep_alive_threshold > self->keep_alive_timer.stop());
                    }

                    NEXTGEN_ATTACH_SHARED_VARIABLES(layer_base, variables_type);
                };

                namespace smtp
                {
                    struct basic_message_variables : public message_base_variables
                    {
                        typedef message_base_variables base_type;

                    };

                    template<typename VariablesType = basic_message_variables>
                    class basic_message : public message_base<VariablesType>
                    {
                        public: typedef VariablesType variables_type;
                        public: typedef byte_array stream_type;
                        public: typedef message_base<variables_type> base_type;

                        public: struct state_type
                        {
                            static const uint32_t none = 0;
                        };

                        NEXTGEN_ATTACH_SHARED_BASE(basic_message, base_type);
                    };

                    template<typename LayerType, typename TransportLayerType, typename MessageType = basic_message<>>
                    struct basic_layer_variables : layer_base_variables<LayerType, TransportLayerType, MessageType>
                    {
                        typedef TransportLayerType transport_layer_type;
                        typedef MessageType message_type;
                        typedef LayerType layer_type;
                        typedef layer_base_variables<layer_type, transport_layer_type, message_type> base_type;

                        typedef typename base_type::service_type service_type;

                        basic_layer_variables(service_type service) : base_type(service)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        basic_layer_variables(transport_layer_type transport_layer) : base_type(transport_layer)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }
                    };

                    template<typename TransportLayerType>
                    class basic_layer : public layer_base<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>, basic_layer_variables<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>>>
                    {
                        public: typedef TransportLayerType transport_layer_type;
                        public: typedef basic_message<> message_type;
                        public: typedef basic_layer_variables<basic_layer<transport_layer_type>, transport_layer_type, message_type> variables_type;
                        public: typedef basic_layer<transport_layer_type> this_type;
                        public: typedef layer_base<this_type, transport_layer_type, message_type, variables_type> base_type;

                        public: typedef typename base_type::base_event_type base_event_type;
                        public: typedef typename base_type::connect_successful_event_type connect_successful_event_type;
                        public: typedef typename base_type::connect_failure_event_type connect_failure_event_type;
                        public: typedef typename base_type::send_successful_event_type send_successful_event_type;
                        public: typedef typename base_type::send_failure_event_type send_failure_event_type;
                        public: typedef std::function<void(message_type)> receive_successful_event_type;
                        public: typedef typename base_type::receive_failure_event_type receive_failure_event_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef typename base_type::accept_failure_event_type accept_failure_event_type;
                        public: typedef typename base_type::host_type host_type;
                        public: typedef typename base_type::port_type port_type;
                        public: typedef typename base_type::keep_alive_threshold_type keep_alive_threshold_type;

                        public: void send_helo(send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            message_type m1;

                            m1->content = "220 localhost\r\n";

                            self.send(m1, successful_handler, failure_handler);
                        }

                        public: void send_bye() const
                        {
                            auto self = *this;

                            message_type m1;
                            m1->content = "221 BYE\r\n";

                            std::cout << "S: " << "221 BYE\r\n" << std::endl;

                            self.send(m1,
                            [=]
                            {
                                self.disconnect();

                                //successful_handler(response);
                            },
                            [=]
                            {
                                //successful_handler(response);
                            });
                        }

                        // bugfix(daemn) lots of copying to get around lambda stack bug
                        public: void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response2;
                            auto response = response2; // bugfix(daemn)

                            std::cout << "S: " << "trying to receive rn" << std::endl;

                            self->transport_layer.receive_until("\r\n", response->stream,
                            [=]
                            {
                                auto self2 = self;
                                auto successful_handler2 = successful_handler;
                                auto failure_handler2 = failure_handler;
                                auto response3 = response; // bugfix(daemn)

                                response3.unpack();

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "Y: " << response3->content << std::endl;

                                if(response3->content.find("EHLO") != std::string::npos)
                                {
                                    std::cout << "S: " << "got a ehlo" << std::endl;

                                    std::string ehlo = nextgen::regex_single_match("EHLO (.+)\r\n", response->content);

                                    std::cout << "S: " << "250-localhost\r\n" << std::endl;
                                    std::cout << "S: " << "250 HELP\r\n" << std::endl;

                                    message_type m1;

                                    m1->content = "250-localhost\r\n"
                                    "250 HELP\r\n"
                                    "\r\n";

                                    self2.send_and_receive(m1, successful_handler2, failure_handler2);
                                }
                                else if(response3->content.find("HELO") != std::string::npos)
                                {
                                    std::cout << "S: " << "got a helo" << std::endl;

                                    self2.receive(successful_handler2, failure_handler2);
                                }
                                else if(response3->content.find("MAIL FROM") != std::string::npos
                                || response3->content.find("VRFY") != std::string::npos
                                || response3->content.find("RCPT TO") != std::string::npos)
                                {
                                    message_type m1;
                                    m1->content = "250 OK\r\n";

                                    std::cout << "S: " << "250 OK\r\n" << std::endl;

                                    self2.send_and_receive(m1, successful_handler2, failure_handler2);
                                }
                                else if(response3->content.find("DATA") != std::string::npos)
                                {
                                    message_type m1;
                                    m1->content = "354 GO\r\n";

                                    std::cout << "S: " << "354 GO\r\n" << std::endl;

                                    self2.send(m1,
                                    [=]
                                    {
                                        auto self3 = self;
                                        auto response4 = response3; // bugfix(daemn)
                                        auto successful_handler3 = successful_handler2;

                                        std::cout << "S: " << "trying to receive rn.rn" << std::endl;

                                        self3->transport_layer.receive_until("\r\n.\r\n", response4->stream,
                                        [=]
                                        {
                                            response4.unpack();

                                            successful_handler3(response4);

                                            self3.send_bye();
                                        },
                                        failure_handler2);
                                    },
                                    failure_handler2);
                                }
                                else if(response3->content.find("QUIT") != std::string::npos)
                                {
                                    self2.send_bye();
                                }
                                else
                                {
                                    std::cout << "<Mail Server> Disconnecting incompatible client." << std::endl;

                                    self2.disconnect();

                                    failure_handler2();
                                }
                            },
                            failure_handler);
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }

                namespace http
                {

                    struct basic_agent_variables
                    {
                        basic_agent_variables(std::string const& title = null) : title(title)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_agent_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                        std::string title;
                    };

                    class basic_agent
                    {
                        NEXTGEN_ATTACH_SHARED_VARIABLES(basic_agent, basic_agent_variables);
                    };

                    class basic_proxy
                    {
                        public: typedef basic_proxy this_type;
                        public: typedef std::string host_type;
                        public: typedef uint32_t port_type;
                        public: typedef uint32_t id_type;
                        public: typedef uint32_t type_type;
                        public: typedef float latency_type;
                        public: typedef timer timer_type;

                        public: struct types
                        {
                            static const uint32_t none = 0;
                            static const uint32_t transparent = 1;
                            static const uint32_t distorting = 2;
                            static const uint32_t anonymous = 3;
                            static const uint32_t elite = 4;
                            static const uint32_t socks4 = 5;
                            static const uint32_t socks5 = 6;
                            static const uint32_t socks4n5 = 7;
                        };

                        public: struct states
                        {
                            static const uint32_t none = 0;
                            static const uint32_t can_only_send = 1;
                            static const uint32_t cannot_send_back = 2;
                            static const uint32_t bad_return_headers = 3;
                            static const uint32_t bad_return_data = 4;
                            static const uint32_t cannot_send = 5;
                            static const uint32_t cannot_connect = 6;
                            static const uint32_t codeen = 7;
                            static const uint32_t perfect = 8;
                            static const uint32_t banned = 9;
                            static const uint32_t invalid = 10;
                            static const uint32_t chunked = 11;
                            //user_agent_via, cache_control, cache_info, connection_close, connection_keep_alive,
                        };

                        private: struct variables
                        {
                            variables(host_type const& host = null, port_type port = null, id_type id = null, type_type type = null, latency_type latency = null) : rating(0), host(host), port(port), id(id), type(0), latency(0.0), state(0), check_delay(6 * 60 * 60)
                            {
                                NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                            }

                            ~variables()
                            {
                                NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                            }

                            int32_t rating;
                            host_type host;
                            port_type port;
                            id_type id;
                            uint32_t type;
                            latency_type latency;
                            timer_type timer;
                            uint32_t state;
                            uint32_t check_delay;
                        };

                        NEXTGEN_ATTACH_SHARED_VARIABLES(basic_proxy, variables);
                    };

                    struct basic_message_variables : public message_base_variables
                    {
                        typedef message_base_variables base_type;
                        typedef std::string raw_header_list_type;
                        typedef boost::unordered_map<std::string, std::string> header_list_type;
                        typedef uint32_t status_code_type;
                        typedef std::string referer_type;
                        typedef boost::unordered_map<std::string, std::string> post_list_type;
                        typedef std::string content_type;
                        typedef std::string path_type;
                        typedef uint32_t id_type;
                        typedef std::string version_type;
                        typedef std::string network_layer_type;
                        typedef std::string host_type;
                        typedef uint32_t port_type;
                        typedef byte_array stream_type;
                        typedef std::string url_type;
                        typedef std::string status_description_type;
                        typedef std::string method_type;

                        basic_message_variables() : base_type(), status_code(0), version("1.1")
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_message_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                        referer_type referer;
                        url_type url;
                        header_list_type header_list;
                        raw_header_list_type raw_header_list;
                        status_code_type status_code;
                        post_list_type post_list;
                        status_description_type status_description;
                        id_type id;
                        method_type method;
                        std::string path;
                        version_type version;
                        network_layer_type address;
                        port_type port;
                        host_type host;
                        std::string username;
                        std::string password;
                        std::string scheme;

                    };

                    template<typename VariablesType = basic_message_variables>
                    class basic_message : public message_base<VariablesType>
                    {
                        typedef VariablesType variables_type;
                        typedef message_base<variables_type> base_type;
                        typedef std::string raw_header_list_type;
                        typedef boost::unordered_map<std::string, std::string> header_list_type;
                        typedef uint32_t status_code_type;
                        typedef std::string referer_type;
                        typedef boost::unordered_map<std::string, std::string> post_list_type;
                        typedef std::string content_type;
                        typedef std::string path_type;
                        typedef uint32_t id_type;
                        typedef std::string version_type;
                        typedef std::string network_layer_type;
                        typedef std::string host_type;
                        typedef uint32_t port_type;
                        typedef byte_array stream_type;
                        typedef std::string url_type;
                        typedef std::string status_description_type;
                        typedef std::string method_type;

                        public: struct state_type
                        {
                            static const uint32_t none = 0;
                            static const uint32_t remove_data_crlf = 1;
                        };

                        public: void pack() const
                        {
                            auto self = *this;

                            std::ostream data_stream(&self->stream.get_buffer());

                            if(self->status_code)
                            {
                                switch(self->status_code)
                                {
                                    case 200: self->status_description = "OK"; break;
                                    default: self->status_description = "UNDEFINED"; break;
                                }

                                std::string response_header = "HTTP/" + self->version + " " + to_string(self->status_code) + " " + self->status_description;

                                if(!self->header_list.empty())
                                // header list already exists
                                {
                                    if(self->username.length() && self->password.length())
                                    // add authentication into header list
                                    {
                                        if(self->header_list.find("Proxy-Authorization") != self->header_list.end())
                                        // authentication header doesn't already exist
                                        {
                                            //self->header_list["Proxy-Authorization"] = "Basic " << base64encode(ps->username + ":" + ps->password);
                                        }
                                    }

                                    // turn header list into raw header string
                                    for(header_list_type::iterator i = self->header_list.begin(), l = self->header_list.end(); i != l; ++i)
                                    {
                                        self->raw_header_list += (*i).first + ": " + (*i).second + "\r\n";
                                    }
                                }

                                self->raw_header_list += "Content-Length: " + to_string(self->content.length()) + "\r\n";

                                if(NEXTGEN_DEBUG_4)
                                {
                                    std::cout << response_header << "\r\n" << std::endl;
                                    std::cout << self->raw_header_list << "\r\n" << std::endl;
                                    std::cout << self->content << std::endl;
                                }

                                data_stream << response_header + "\r\n";
                                data_stream << self->raw_header_list + "\r\n";
                                data_stream << self->content;
                            }
                            else if(self->method.length())
                            {
                                if(!self->post_list.empty())
                                // parse post list
                                {

                                }

                                auto header_list = self->header_list;
                                std::string raw_header_list = "";
                                std::string content = self->content;

                                header_list_type::iterator i, l;

                                std::string request_header = self->method + " " + self->url + " " + "HTTP" + "/" + self->version;

                                if(header_list.empty())
                                // turn raw header string into a header list
                                {

                                }
                                else
                                // header list already exists
                                {

                                    bool content_length_exists;

                                    if(header_list.find("Content-Length") != header_list.end())
                                        content_length_exists = true;
                                    else
                                        content_length_exists = false;


                                    if(self->username.length() && self->password.length())
                                    // add authentication into header list
                                    {
                                        if(header_list.find("Proxy-Authorization") != header_list.end())
                                        // authentication header doesn't already exist
                                        {
                                            //self->header_list["Proxy-Authorization"] = "Basic " << base64encode(ps->username + ":" + ps->password);
                                        }
                                    }

                                    if((i = header_list.find("Host")) != header_list.end())
                                    {
                                        raw_header_list = (*i).first + ": " + (*i).second  + "\r\n" + raw_header_list;

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("User-Agent")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        // we're at the user agent header, so let's tell them what encoding our agent accepts
                                        raw_header_list += "Accept: */*" "\r\n"; //text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
                                        raw_header_list += "Accept-Language: en-us,en;q=0.5" "\r\n";
                                        raw_header_list += "Accept-Encoding: gzip,deflate" "\r\n";
                                        raw_header_list += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7" "\r\n";

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("Keep-Alive")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        header_list.erase(i);
                                    }

                                    if((i = header_list.find("Connection")) != header_list.end())
                                    {
                                        raw_header_list += (*i).first + ": " + (*i).second  + "\r\n";

                                        header_list.erase(i);
                                    }

                                    // turn header list into raw header string
                                    for(i = header_list.begin(), l = header_list.end(); i != l; ++i)
                                    {
                                        auto header_title = (*i).first;
                                        auto header_value = (*i).second;

                                        raw_header_list += header_title + ": " + header_value + "\r\n";
                                    }

                                    if(self->method == "POST" && !content_length_exists)
                                    {
                                        raw_header_list += "Content-Type: application/x-www-form-urlencoded" "\r\n";
                                    // user didn't specify a content length, so we'll count it for them
                                        raw_header_list += "Content-Length: " + to_string(content.length()) + "\r\n";
                                    }
                                }

                                if(NEXTGEN_DEBUG_4)
                                {
                                    std::cout << request_header << std::endl;
                                    std::cout << raw_header_list << std::endl;
                                    std::cout << content << std::endl;
                                }

                                data_stream << request_header + "\r\n";
                                data_stream << raw_header_list + "\r\n";
                                data_stream << content;
                            }
                        }

                        public: void unpack_headers() const
                        {
                            auto self = *this;
std::cout << "http message unpack headers" << std::endl;
                            if(NEXTGEN_DEBUG_2)
                                std::cout << "XXXXXX" << self->stream.get_buffer().in_avail() << std::endl;

                            std::istream data_stream(&self->stream.get_buffer());

                            std::string data((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                            size_t header_end = data.find("\r\n\r\n");

                            if(header_end != std::string::npos)
                            {
                                self->raw_header_list += data.substr(0, header_end + 2);

                                data.erase(0, header_end + 4);

                                self->content = data;
                            }


                            std::string line;
                            getline(self->raw_header_list, line);

                            if(NEXTGEN_DEBUG_4)
                                std::cout << "UNPACKING LINE: " << line << std::endl;

                            if(NEXTGEN_DEBUG_2)
                                std::cout << "AFTER UNPACKING LINE: " << self->raw_header_list << std::endl;

                            boost::regex_error paren(boost::regex_constants::error_paren);

                            try
                            {
                                boost::match_results<std::string::const_iterator> what;
                                boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                                std::string::const_iterator start = line.begin();
                                std::string::const_iterator end = line.end();

                                // todo(daemn) fix line bug
                                if(boost::regex_search(start, end, what, boost::regex("^HTTP\\/([^ ]+) ([0-9]+) (.+?)"), flags))
                                {
                                    if(NEXTGEN_DEBUG_2)
                                        std::cout << "ZZZZZZZZZZ" << what[1] << what[2] << what[3] << std::endl;

                                    self->version = what[1];
                                    self->status_code = to_int(what[2]);
                                    self->status_description = what[3];
                                    boost::to_lower(self->status_description);
                                }
                                else
                                {
                                    if(boost::regex_search(start, end, what, boost::regex("^([^ ]+) ([^ ]+) HTTP\\/([^ ]+)"), flags))
                                    {
                                        if(NEXTGEN_DEBUG_2)
                                            std::cout << "YYYYYYYY " << what[1] << what[2] << what[3] << std::endl;

                                        self->method = what[1];
                                        self->path = what[2];
                                        self->version = what[3];
                                    }
                                    else
                                    {
                                        if(NEXTGEN_DEBUG_5)
                                            std::cout << "Error unpacking HTTP header." << std::endl;
                                    }
                                }
                            }
                            catch(boost::regex_error const& e)
                            {
                                std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
                            }

                            self->content = data;

                            self->header_list.clear();

                            try
                            {
                                boost::match_results<std::string::const_iterator> what;
                                boost::match_flag_type flags = boost::regex_constants::match_perl | boost::regex_constants::format_perl;

                                std::string::const_iterator start = self->raw_header_list.begin();
                                std::string::const_iterator end = self->raw_header_list.end();

                                self->header_list["set-cookie"] = "";

                                while(boost::regex_search(start, end, what, boost::regex("(.+?)\\: (.+?)\r\n"), flags))
                                {
                                    if(what[1].length() > 0)
                                    {
                                        std::string key = what[1];

                                        boost::to_lower(key);

                                        if(NEXTGEN_DEBUG_2)
                                            std::cout << "K: " << key << ": " << what[2] << std::endl;

                                        if(key == "set-cookie")
                                        {
                                            self->header_list[key] += what[2] + " ";
                                        }
                                        else
                                            self->header_list[key] = what[2];
                                    }

                                    // update search position:
                                    start = what[0].second;

                                    // update flags:
                                    flags |= boost::match_prev_avail;
                                    flags |= boost::match_not_bob;
                                }
                            }
                            catch(boost::regex_error const& e)
                            {
                                std::cout << "regex error: " << (e.code() == paren.code() ? "unbalanced parentheses" : "?") << std::endl;
                            }

                            if(self->status_code != 0)
                            // we're unpacking response headers
                            {


                                // todo(daemn) look for content-length and set-cookie and content-type
                            }
                            else
                            // we're unpacking request headers
                            {

                            }
                        }

                        public: void unpack_content() const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_4)
                            {
                                std::cout << "LEN!! " << self->stream.get_buffer().in_avail() << std::endl;
                                //std::cout << "LEN2!! " << self->raw_header_list.length() << std::endl;
                                //std::cout << "LEN3!! " << self->raw_header_list << std::endl;
                            }

                            std::istream data_stream(&self->stream.get_buffer());

                            self->content += std::string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());
std::cout << "encoding: " << self->header_list["content-encoding"] << std::endl;
std::cout << "size: " << self->content.size() << std::endl;
                            if(self->header_list["content-encoding"] == "gzip"
                            && self->content.size() > 0)
                            {
                                std::vector<char> buffer;
                                std::string error;

                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "unpacking compressed l: " << self->content.length() << std::endl;

                                if(inflate_gzip(self->content.data(), self->content.length(), buffer, 1024 * 1024, error))
                                {
                                    std::cout << "Error uncompressing: " << error << std::endl;
                                }

                                if(buffer.size() > 0)
                                    self->content = &buffer[0];
                                else
                                    std::cout << "No content after uncompression" << std::endl;
                            }
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_message, base_type);
                    };

                    template<typename LayerType, typename TransportLayerType, typename MessageType = basic_message<>>
                    struct basic_layer_variables : layer_base_variables<LayerType, TransportLayerType, MessageType>
                    {
                        typedef TransportLayerType transport_layer_type;
                        typedef MessageType message_type;
                        typedef LayerType layer_type;
                        typedef layer_base_variables<layer_type, transport_layer_type, message_type> base_type;

                        typedef typename base_type::service_type service_type;
                        typedef basic_proxy proxy_type;

                        basic_layer_variables(service_type service) : base_type(service), proxy(null)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        basic_layer_variables(transport_layer_type transport_layer) : base_type(transport_layer), proxy(null)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }

                        proxy_type proxy;
                    };

                    template<typename TransportLayerType>
                    class basic_layer : public layer_base<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>, basic_layer_variables<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>>>
                    {
                        public: typedef TransportLayerType transport_layer_type;
                        public: typedef basic_message<> message_type;
                        public: typedef basic_layer_variables<basic_layer<transport_layer_type>, transport_layer_type, message_type> variables_type;
                        public: typedef basic_layer<transport_layer_type> this_type;
                        public: typedef layer_base<this_type, transport_layer_type, message_type, variables_type> base_type;

                        public: typedef typename base_type::base_event_type base_event_type;
                        public: typedef typename base_type::connect_successful_event_type connect_successful_event_type;
                        public: typedef typename base_type::connect_failure_event_type connect_failure_event_type;
                        public: typedef typename base_type::send_successful_event_type send_successful_event_type;
                        public: typedef typename base_type::send_failure_event_type send_failure_event_type;
                        public: typedef std::function<void(message_type)> receive_successful_event_type;
                        public: typedef typename base_type::receive_failure_event_type receive_failure_event_type;
                        public: typedef typename base_type::host_type host_type;
                        public: typedef typename base_type::port_type port_type;
                        public: typedef typename base_type::keep_alive_threshold_type keep_alive_threshold_type;
                        public: typedef typename variables_type::proxy_type proxy_type;


                        public: void reconnect(connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            if(NEXTGEN_DEBUG_5)
                                std::cout << "[nextgen::network::http_client] reconnecting" << std::endl;

                            self.disconnect();
                            self.connect(self->host, self->port, successful_handler2, failure_handler2);
                        }

                        public: void connect(host_type const& host_, port_type port_, connect_successful_event_type successful_handler2 = 0, connect_failure_event_type failure_handler2 = 0) const
                        {
                            auto self = *this;

                            auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                            auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                            if(successful_handler == 0)
                                successful_handler = self->connect_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->connect_failure_event;

                            self->host = host_;
                            self->port = port_;

                            std::string host;
                            uint32_t port;

                            if(self->proxy != 0)
                            {
                                host = self->proxy->host;
                                port = self->proxy->port;

                                //self->proxy_address = self->proxy;
                            }
                            else
                            {
                                host = self->host;
                                port = self->port;
                            }

                            self->transport_layer = transport_layer_type(self->transport_layer->service);

                            self->transport_layer.connect(host, port,
                            [=]
                            {
                                if(NEXTGEN_DEBUG_4)
                                    std::cout << "[nextgen::network::http_client] Connected" << std::endl;


                                //if(self->proxy ==proxy_type::types::transparent
                                //|| self->proxy ==proxy_type::types::distorting
                                //|| self->proxy ==proxy_type::types::anonymous)
                                if(self->proxy->type == proxy_type::types::socks4
                                || self->proxy->type == proxy_type::types::socks4n5)
                                {
                                    hostent* host_entry = gethostbyname(self->host.c_str());

                                    if(host_entry == NULL)
                                    {
                                        self.disconnect();

                                        failure_handler();

                                        return;
                                    }

                                    std::string addr = inet_ntoa(*(in_addr*)*host_entry->h_addr_list);

                                    byte_array r1;

                                    r1 << (byte)4;
                                    r1 << (byte)1;
                                    r1 << htons(self->port);
                                    r1 << inet_addr(addr.c_str());
                                    r1 << "PRO";

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << r1.to_string() << std::endl;

                                    self->transport_layer.send(r1,
                                    [=]
                                    {
                                        if(NEXTGEN_DEBUG_4)
                                            std::cout << "sent socks4 request" << std::endl;

                                        byte_array r2;

                                        self->transport_layer.receive(asio::transfer_at_least(8), r2,
                                        [=]
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "received socks4 response" << std::endl;

                                            byte none;
                                            byte status;

                                            r2 >> none;
                                            r2 >> status;

                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << r2.to_string() << std::endl;

                                            switch(status)
                                            {
                                                case 0x5a:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "is valid socks4 response" << std::endl;

                                                    successful_handler();
                                                }
                                                break;

                                                default:
                                                {
                                                    if(NEXTGEN_DEBUG_4)
                                                        std::cout << "failed2 socks4 response" << std::endl;

                                                    self.disconnect();

                                                    failure_handler();
                                                }
                                            }
                                        },
                                        failure_handler);
                                    },
                                    failure_handler);
                                }
                                else if(self->proxy->type == proxy_type::types::socks5)
                                {
                                    byte_array r1;

                                    r1 << (byte)5;
                                    r1 << (byte)1;
                                    r1 << (byte)0;

                                    if(NEXTGEN_DEBUG_4)
                                        std::cout << r1.to_string() << std::endl;

                                    self->transport_layer.send(r1,
                                    [=]
                                    {
                                        if(NEXTGEN_DEBUG_4)
                                            std::cout << "sent socks5 request" << std::endl;

                                        byte_array r2;

                                        self->transport_layer.receive(asio::transfer_at_least(2), r2,
                                        [=]
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "received socks5 response" << std::endl;

                                            byte status;
                                            byte status2;

                                            r2 >> status;
                                            r2 >> status2;

                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << r2.to_string() << std::endl;

                                            if(status == 0x05 && status2 == 0x00)
                                            {
                                                hostent* host_entry = gethostbyname(self->host.c_str());
                                                std::string addr = inet_ntoa(*(in_addr*)*host_entry->h_addr_list);

                                                if(host_entry == NULL)
                                                {
                                                    self.disconnect();

                                                    failure_handler();

                                                    return;
                                                }

                                                byte_array r3;

                                                r3 << (byte)5;
                                                r3 << (byte)1;
                                                r3 << (byte)0;
                                                r3 << (byte)1;
                                                r3 << inet_addr(addr.c_str());
                                                r3 << htons(self->port);

                                                if(NEXTGEN_DEBUG_4)
                                                    std::cout << r3.to_string() << std::endl;

                                                self->transport_layer.send(r3,
                                                [=]()
                                                {
                                                    byte_array r4;

                                                    self->transport_layer.receive(asio::transfer_at_least(1), r4,
                                                    [=]()
                                                    {
                                                        byte nothing;
                                                        byte address_type;

                                                        r4 << nothing;
                                                        r4 << address_type;

                                                        // todo(daemn) we've gotten this far, assume we're good.
                                                        successful_handler();
                                                    },
                                                    failure_handler);
                                                },
                                                failure_handler);
                                            }
                                            else
                                            {
                                                self.disconnect();

                                                failure_handler();
                                            }
                                        },
                                        failure_handler);
                                    },
                                    failure_handler);
                                }
                                else
                                {
                                    successful_handler();
                                }
                            },
                            failure_handler);
                        }

                        public: void send(message_type request, send_successful_event_type successful_handler = 0, send_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(request->url.find("https") != std::string::npos
                            && !self->transport_layer->ssl)
                            {
                                message_type m1;
                                m1->version = "1.1";
                                m1->method = "CONNECT";
                                m1->url = self->host + ":443";
                                m1->header_list = request->header_list;

                                self.send_and_receive(m1,
                                [=](message_type r1)
                                {
                                    if(r1->status_code == 200)
                                    {
                                        self->transport_layer->ssl = true;

                                        self->transport_layer->ssl_socket.async_handshake(asio::ssl::stream_base::client,
                                        [=](asio::error_code const& error)
                                        {
                                            if(!error)
                                            {
                                                request.pack();

                                                self.send_stream(request->stream, successful_handler, failure_handler);
                                            }
                                            else
                                            {
                                                if(NEXTGEN_DEBUG_4)
                                                    std::cout << "<http_socket::send handler> Error: " << error.message() << std::endl;

                                                self.disconnect();

                                                failure_handler();
                                            }
                                        });
                                    }
                                    else
                                    {
                                        self.disconnect();

                                        failure_handler();
                                    }
                                }, failure_handler);
                            }
                            else
                            {
                                request.pack();

                                self.send_stream(request->stream, successful_handler, failure_handler);
                            }
                        }

                        private: void receive_chunked_data(message_type response, size_t length = 1, base_event_type successful_handler = 0, base_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            std::cout << "receive_chunked_data" << std::endl;

                            self->transport_layer.receive(asio::transfer_at_least(length), response->stream,
                            [=]
                            {
                                std::istream data_stream(&response->stream.get_buffer());

                                response->content += std::string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                                        std::cout << "CONTENT L BEFORE: " << response->content.size() << std::endl;

                                        //std::cout << "CONTENT BEFORE: " <<  to_hex(response->content) << std::endl;

                                //int l = static_cast<int>(strtol(length, NULL, 16));

                                uint32_t length;
                                size_t pos;
                                std::string data;

                                while(true)
                                {
                                    if(to_int(response->header_list["content-length"]) >= (int)response->content.size())
                                    {
                                        size_t recv_amount = to_int(response->header_list["content-length"]) - response->content.size();

                                        if(recv_amount == 0) recv_amount = 1;

                                        self.receive_chunked_data(response, recv_amount, successful_handler, failure_handler);
                                    }
                                    else if(to_int(response->header_list["content-length"]) < (int)response->content.size())
                                    {
                                        if(response->state == message_type::state_type::remove_data_crlf)
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);

                                            response->state = message_type::state_type::none;
                                        }

                                        pos = response->content.find("\r\n", to_int(response->header_list["content-length"]));

                                        if(pos == std::string::npos)
                                        {
                                            self.disconnect();

                                            failure_handler();

                                            return;
                                        }

                                        std::cout << "getline_intern " << pos << std::endl;

                                        data = response->content.substr(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]));

                                        response->content.erase(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]) + 2);

                                        length = read_hex(data.c_str());

                                        std::cout << "chunked length hex: " << data.c_str() << std::endl;
                                        std::cout << "chunked length: " << length << std::endl;

                                        response->header_list["content-length"] = to_string(to_int(response->header_list["content-length"]) + length);

                                        if((int)response->content.size() >= (to_int(response->header_list["content-length"]) + 2))
                                        {
                                            // erase newline
                                            response->content.erase(to_int(response->header_list["content-length"]), 2);
                                        }
                                        else
                                        {
                                            response->state = message_type::state_type::remove_data_crlf;
                                        }

                                        std::cout << "CHUNK: " << to_int(response->header_list["content-length"]) << " / " << response->content.size() << std::endl;

                                        if(length == 0)
                                        {
                                            //response->content.erase(response->content.size() - 2, 2); //blank line

                                            //std::cout << "after: " << to_hex(response->content) << std::endl;

                                            successful_handler();
                                        }
                                        else
                                            continue;
                                    }

                                    break;
                                }
                            },
                            failure_handler);
                        }

                        public: void receive(receive_successful_event_type successful_handler = 0, receive_failure_event_type failure_handler = 0) const
                        {
                            auto self = *this;

                            if(successful_handler == 0)
                                successful_handler = self->receive_successful_event;

                            if(failure_handler == 0)
                                failure_handler = self->receive_failure_event;

                            message_type response2;
                            auto response = response2; // bugfix(daemn)

                            self->transport_layer.receive_until("\r\n\r\n", response->stream,
                            [=]
                            {
                                response.unpack_headers();

                                if(NEXTGEN_DEBUG_3)
                                    std::cout << "Z: " << response->raw_header_list << std::endl;

                                if(response->status_code != 0 || response->method == "POST")
                                // this http message has content, and we need to know the length
                                {
                                    if(response->header_list.find("transfer-encoding") != response->header_list.end()
                                    &&response->header_list["transfer-encoding"] == "chunked")
                                    {
                                        std::cout << "unchunking encoding" << std::endl;

                                        std::istream data_stream(&response->stream.get_buffer());

                                        response->header_list["content-length"] = "0";

                                        response->content += std::string((std::istreambuf_iterator<char>(data_stream)), std::istreambuf_iterator<char>());

                                        std::cout << "CONTENT L BEFORE: " << response->content.size() << std::endl;

                                        std::cout << "CONTENT BEFORE: " <<  to_hex(response->content) << std::endl;

                                        uint32_t length;
                                        size_t pos;
                                        std::string data;

                                        while(true)
                                        {
                                            if(to_int(response->header_list["content-length"]) >= (int)response->content.size())
                                            {
                                                size_t recv_amount = to_int(response->header_list["content-length"]) - response->content.size();

                                                if(recv_amount == 0) recv_amount = 1;

                                                self.receive_chunked_data(response, recv_amount, [=]
                                                {
                                                    response.unpack_content();

                                                    successful_handler(response);
                                                },
                                                [=]
                                                {
                                                    if(NEXTGEN_DEBUG_5)
                                                        std::cout << "failed to receive rest of chunked encoding" << std::endl;

                                                    self.disconnect();

                                                    failure_handler();
                                                });
                                            }
                                            else if(to_int(response->header_list["content-length"]) < (int)response->content.size())
                                            {
                                                if(response->state == message_type::state_type::remove_data_crlf)
                                                {
                                                    // erase newline
                                                    response->content.erase(to_int(response->header_list["content-length"]), 2);

                                                    response->state = message_type::state_type::none;
                                                }

                                                pos = response->content.find("\r\n", to_int(response->header_list["content-length"]));

                                                if(pos == std::string::npos)
                                                {
                                                    self.disconnect();

                                                    failure_handler();

                                                    return;
                                                }

                                                std::cout << "getline_intern " << pos << std::endl;

                                                data = response->content.substr(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]));

                                                response->content.erase(to_int(response->header_list["content-length"]), pos - to_int(response->header_list["content-length"]) + 2);

                                                length = read_hex(data.c_str());

                                                std::cout << "chunked length hex: " << data.c_str() << std::endl;
                                                std::cout << "chunked length: " << length << std::endl;

                                                response->header_list["content-length"] = to_string(to_int(response->header_list["content-length"]) + length);

                                                if((int)response->content.size() >= (to_int(response->header_list["content-length"]) + 2))
                                                {
                                                    // erase newline
                                                    response->content.erase(to_int(response->header_list["content-length"]), 2);
                                                }
                                                else
                                                {
                                                    response->state = message_type::state_type::remove_data_crlf;
                                                }

                                                std::cout << "CHUNK: " << to_int(response->header_list["content-length"]) << " / " << response->content.size() << std::endl;

                                                if(length == 0)
                                                {
                                                    //response->content.erase(response->content.size() - 2, 2); //blank line


                                                    std::cout << "after: " << to_hex(response->content) << std::endl;

                                                    if(self->proxy != null_str
                                                    && response->content.find("Content-Type") != std::string::npos)
                                                    {
                                                        // this proxy wrapped the headers in the content

                                                        response->raw_header_list = response->content;
                                                        response.unpack_headers();
                                                        response.unpack_content();

                                                    }

                                                    successful_handler(response);
                                                }
                                                else
                                                    continue;
                                            }

                                            break;
                                        }
                                    }
                                    else if(response->header_list.find("content-length") != response->header_list.end())
                                    {
                                        if(NEXTGEN_DEBUG_3)
                                            std::cout << "VVVVVVVVVVVVV " << response->header_list["content-length"] << " VVVVVV " << response->header_list["Content-Length"].length() << std::endl;

                                        auto content_length = to_int(response->header_list["content-length"]) - response->content.length();

                                        if(content_length == 0)
                                        {
                                            successful_handler(response);
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_4)
                                                std::cout << "trying to receive length = " << content_length << std::endl;

                                            self->transport_layer.receive(asio::transfer_at_least(content_length), response->stream,
                                            [=]
                                            {
                                                response.unpack_content();

                                                successful_handler(response);
                                            },
                                            [=]
                                            {
                                                if(NEXTGEN_DEBUG_5)
                                                    std::cout << "failed to receive rest of length but still gonna success" << std::endl;

                                                response.unpack_content();

                                                successful_handler(response);
                                            });
                                        }
                                    }
                                    else
                                    {
                                        if(response->status_code == 204)
                                        {
                                            if(NEXTGEN_DEBUG_5)
                                                std::cout << "No http content-length specified due to 204" << std::endl;

                                            response->header_list["content-length"] = "0";

                                            response.unpack_content();

                                            successful_handler(response);
                                        }
                                        else if(response->status_description == "connection established")
                                        {
                                            successful_handler(response);
                                        }
                                        else
                                        {
                                            if(NEXTGEN_DEBUG_5)
                                                std::cout << "No http content-length specified so assuming its auto EOF" << std::endl;

                                            self->transport_layer.receive(asio::transfer_at_least(1), response->stream,
                                            [=]
                                            {
                                                response.unpack_content();

                                                successful_handler(response);
                                            },
                                            [=]
                                            {
                                                if(NEXTGEN_DEBUG_5)
                                                    std::cout << "failed to receive after no content-length" << std::endl;

                                                failure_handler();
                                            });
                                        }
                                    }
                                }
                                else
                                // this http message just contains raw headers
                                {
                                    if(response->raw_header_list.length() > 0)
                                        successful_handler(response);
                                    else
                                    {
                                        self.disconnect();

                                        failure_handler();
                                    }
                                }
                            },
                            failure_handler);
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }

                namespace xml
                {
                    struct basic_message_variables : public message_base_variables
                    {
                        typedef message_base_variables base_type;

                    };

                    template<typename VariablesType = basic_message_variables>
                    class basic_message : public message_base<VariablesType>
                    {
                        typedef VariablesType variables_type;
                        typedef message_base<variables_type> base_type;

                        NEXTGEN_ATTACH_SHARED_BASE(basic_message, base_type);
                    };

                    template<typename LayerType, typename TransportLayerType, typename MessageType = basic_message<>>
                    struct basic_layer_variables : layer_base_variables<LayerType, TransportLayerType, MessageType>
                    {
                        typedef TransportLayerType transport_layer_type;
                        typedef MessageType message_type;
                        typedef LayerType layer_type;
                        typedef layer_base_variables<layer_type, transport_layer_type, message_type> base_type;

                        typedef typename base_type::service_type service_type;

                        basic_layer_variables(service_type service) : base_type(service)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        basic_layer_variables(transport_layer_type transport_layer) : base_type(transport_layer)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }
                    };

                    template<typename TransportLayerType>
                    class basic_layer : public layer_base<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>, basic_layer_variables<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>>>
                    {
                        public: typedef TransportLayerType transport_layer_type;
                        public: typedef basic_message<> message_type;
                        public: typedef basic_layer_variables<basic_layer<transport_layer_type>, transport_layer_type, message_type> variables_type;
                        public: typedef basic_layer<transport_layer_type> this_type;
                        public: typedef layer_base<this_type, transport_layer_type, message_type, variables_type> base_type;

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }

                namespace ngp
                {
                    struct basic_message_variables : public message_base_variables
                    {
                        typedef message_base_variables base_type;


                        uint32_t id;
                        uint32_t length;
                        byte_array data;
                    };

                    template<typename VariablesType = basic_message_variables>
                    class basic_message : public message_base<VariablesType>
                    {
                        typedef VariablesType variables_type;
                        typedef message_base<variables_type> base_type;

                        public: void pack() const
                        {
                            auto self = *this;

                            self->stream << self->id << self->data.length() << self->data;
                        }

                        public: void unpack() const
                        {
                            auto self = *this;

                            self->stream->little_endian = true;

                            if(self->stream.available() >= 8)
                            {
                                self->stream >> self->id;
                                self->stream >> self->length;

                                if(self->stream.available() >= self->length)
                                {
                                    self->data = byte_array(self->stream, self->length);
                                }
                            }
                        }

                        NEXTGEN_ATTACH_SHARED_BASE(basic_message, base_type);
                    };

                    template<typename LayerType, typename TransportLayerType, typename MessageType = basic_message<>>
                    struct basic_layer_variables : layer_base_variables<LayerType, TransportLayerType, MessageType>
                    {
                        typedef TransportLayerType transport_layer_type;
                        typedef MessageType message_type;
                        typedef LayerType layer_type;
                        typedef layer_base_variables<layer_type, transport_layer_type, message_type> base_type;

                        typedef typename base_type::service_type service_type;

                        basic_layer_variables(service_type service) : base_type(service)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        basic_layer_variables(transport_layer_type transport_layer) : base_type(transport_layer)
                        {
                            NEXTGEN_DEBUG_CONSTRUCTOR(*this);
                        }

                        ~basic_layer_variables()
                        {
                            NEXTGEN_DEBUG_DECONSTRUCTOR(*this);
                        }
                    };

                    template<typename TransportLayerType>
                    class basic_layer : public layer_base<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>, basic_layer_variables<basic_layer<TransportLayerType>, TransportLayerType, basic_message<>>>
                    {
                        public: typedef TransportLayerType transport_layer_type;
                        public: typedef basic_message<> message_type;
                        public: typedef basic_layer_variables<basic_layer<transport_layer_type>, transport_layer_type, message_type> variables_type;
                        public: typedef basic_layer<transport_layer_type> this_type;
                        public: typedef layer_base<this_type, transport_layer_type, message_type, variables_type> base_type;

                        public: typedef typename base_type::base_event_type base_event_type;
                        public: typedef typename base_type::connect_successful_event_type connect_successful_event_type;
                        public: typedef typename base_type::connect_failure_event_type connect_failure_event_type;
                        public: typedef typename base_type::send_successful_event_type send_successful_event_type;
                        public: typedef typename base_type::send_failure_event_type send_failure_event_type;
                        public: typedef std::function<void(message_type)> receive_successful_event_type;
                        public: typedef typename base_type::receive_failure_event_type receive_failure_event_type;
                        public: typedef std::function<void(this_type)> accept_successful_event_type;
                        public: typedef typename base_type::accept_failure_event_type accept_failure_event_type;
                        public: typedef typename base_type::host_type host_type;
                        public: typedef typename base_type::port_type port_type;
                        public: typedef typename base_type::keep_alive_threshold_type keep_alive_threshold_type;

                        NEXTGEN_ATTACH_SHARED_BASE(basic_layer, base_type);
                    };
                }
            }
        }

        typedef ip::application::smtp::basic_message<> smtp_message;
        typedef ip::application::smtp::basic_layer<tcp_socket> smtp_client;

        typedef ip::application::http::basic_message<> http_message;
        typedef ip::application::http::basic_layer<tcp_socket> http_client;
        typedef ip::application::http::basic_agent http_agent;
        typedef ip::application::http::basic_proxy http_proxy;

        typedef ip::application::xml::basic_message<> xml_message;
        typedef ip::application::xml::basic_layer<tcp_socket> xml_client;

        typedef ip::application::ngp::basic_message<> ngp_message;
        typedef ip::application::ngp::basic_layer<tcp_socket> ngp_client;


        template<typename layer>
        class server_base
        {
            public: typedef layer client_type;
            public: typedef uint32_t port_type;

            public: typedef std::function<void()> base_event_type;
            public: typedef std::function<void(client_type)> accept_successful_event_type;
            public: typedef base_event_type accept_failure_event_type;
        };

        template<typename LayerType>
        class basic_server : public server_base<LayerType> // todo(daemn) inheriting doesnt seem to be working
        {
            public: typedef basic_service<> service_type;
            public: typedef LayerType layer_type;
            public: typedef layer_type server_type;
            public: typedef layer_type client_type;
            public: typedef std::list<client_type> client_list_type;
            public: typedef uint32_t port_type;
            public: typedef std::function<void()> base_event_type;
            public: typedef std::function<void(client_type)> accept_successful_event_type;
            public: typedef base_event_type accept_failure_event_type;

            public: void accept(accept_successful_event_type successful_handler2 = 0, accept_failure_event_type failure_handler2 = 0)
            {
                auto self2 = *this;
                auto self = self2;

                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen:network:server:accept] Waiting for client..." << std::endl;

                auto successful_handler = successful_handler2; // bugfix(daemn) gah!!
                auto failure_handler = failure_handler2; // bugfix(daemn) gah!!

                if(successful_handler == 0)
                    successful_handler = self->accept_successful_event;

                if(failure_handler == 0)
                    failure_handler = self->accept_failure_event;

                self->server.accept(self->port,
                [=](client_type client)
                {
                    auto self3 = self; // bugfix(daemn) wow..... no stack

                    if(NEXTGEN_DEBUG_2)
                        std::cout << "[nextgen::network::server::accept] Successfully accepted client." << std::endl;

                    self->client_list.push_back(client);

                    client->keep_alive_threshold = 240;

                    client->disconnect_event += [=]()
                    {
                        if(NEXTGEN_DEBUG_4)
                            std::cout << "ERASE SERVER CLIENT" << std::endl;

                        self3.remove_client(client);
                    };

                    client->transport_layer->close_event += client->disconnect_event; // temp(daemn) event not saving callback refs or something

                    successful_handler(client);
                },
                [=]
                {
                    if(NEXTGEN_DEBUG_2)
                        std::cout << "[nextgen::network::server::accept] Failed to accept client." << std::endl;

                    failure_handler();
                });
            }

            public: void remove_client(client_type client) const
            {
                auto self = *this;

                auto i = std::find(self->client_list.begin(), self->client_list.end(), client);

                if(i != self->client_list.end())
                    self->client_list.erase(i);
            }

            public: void disconnect()
            {
                auto self = *this;

                std::for_each(self->client_list.begin(), self->client_list.end(), [=](client_type& client)
                {
                    client.disconnect();

                    if(NEXTGEN_DEBUG_4)
                        std::cout << "DISCONNECT SERVER CLIENT" << std::endl;
                });

                self->client_list.reset();
            }

            public: void clean()
            {
                auto self = *this;

                if(NEXTGEN_DEBUG_4)
                    std::cout << "[nextgen:server] Cleaning out expired clients.";

                std::remove_if(self->client_list.begin(), self->client_list.end(), [=](client_type& client) -> bool
                {
                    if(client.is_alive())
                    {
                        return false;
                    }
                    else
                    {
                        if(NEXTGEN_DEBUG_4)
                            std::cout << ".";

                        client.disconnect();

                        return true;
                    }
                });

                if(NEXTGEN_DEBUG_4)
                    std::cout << std::endl;
            }

            private: struct variables
            {
                variables(service_type service, port_type port) : service(service), server(service), port(port)
                {

                }

                event<accept_failure_event_type> accept_failure_event;
                event<accept_successful_event_type> accept_successful_event;

                service_type service;
                server_type server;
                port_type port;
                client_list_type client_list;
                boost::unordered_map<std::string, nextgen::event<std::function<void(std::string)>>> handler_list;
            };

            NEXTGEN_ATTACH_SHARED_VARIABLES(basic_server, variables);
        };

        typedef basic_server<http_client> http_server;
        typedef basic_server<smtp_client> smtp_server;
        typedef basic_server<xml_client> xml_server;
        typedef basic_server<ngp_client> ngp_server;

        template<typename layer_type>
        void create_server(basic_service<> service, uint32_t port, std::function<void(layer_type)> successful_handler = 0, std::function<void()> failure_handler = 0)
        {
            if(NEXTGEN_DEBUG_2)
                std::cout << "[nextgen:network:server:accept] Waiting for client..." << std::endl;

            layer_type server(service);

            server.accept(port,
            [=](layer_type client)
            {
                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen::network::server::accept] Successfully accepted client." << std::endl;

                if(successful_handler != 0)
                    successful_handler(client);
            },
            [=]()
            {
                if(NEXTGEN_DEBUG_2)
                    std::cout << "[nextgen::network::server::accept] Failed to accept client." << std::endl;

                if(failure_handler != 0)
                    failure_handler();
            });
        }

        typedef basic_service<> service;
    }

    // todo(daemn) cant make this a template - ice segfault
    void timeout(network::basic_service<> service, std::function<void()> callback, uint32_t milliseconds)
    {
        if(milliseconds > 0)
        {
            if(NEXTGEN_DEBUG_2)
                std::cout << "timeout for " << milliseconds << std::endl;

            boost::shared_ptr<asio::deadline_timer> timer(new asio::deadline_timer(service.get_service()));

            timer->expires_from_now(boost::posix_time::milliseconds(milliseconds));

            timer->async_wait([=](asio::error_code const& error)
            {
                timer->expires_from_now(); // bugfix(daemn) it's going to go out of scope and cancel the timer automatically

                callback();
            });
        }
        else
        {
            callback();
        }
    }
}

#endif

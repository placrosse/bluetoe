#ifndef BLUETOE_LINK_LAYER_ADDRESS_HPP
#define BLUETOE_LINK_LAYER_ADDRESS_HPP

#include <cstdint>
#include <initializer_list>
#include <iosfwd>

namespace bluetoe {
namespace link_layer {

    class random_device_address;

    /**
     * @brief a 48-bit universal LAN MAC address
     */
    class address
    {
    public:
        /**
         * @brief creates the address with all octes beeing zero (00:00:00:00:00:00)
         */
        address();

        /**
         * @brief initialize an address by a initializer list with exactly 6 elements
         */
        explicit address( const std::initializer_list< std::uint8_t >& initial_values );

        /**
         * @brief initializing an address by taking 6 bytes from the given start of an array
         */
        explicit address( const std::uint8_t* initial_values );

        /**
         * @brief generates a valid static random address out of a given seed value
         *
         * The function generates a random device address that is valid and that has always
         * the same value for the same seed.
         */
        static random_device_address generate_static_random_address( std::uint32_t seed );

        /**
         * @brief prints this in a human readable manner
         */
        std::ostream& print( std::ostream& ) const;

        /**
         * @brief returns the most significant byte of the address
         */
        std::uint8_t msb() const;

        /**
         * @brief returns true, if this address is the same as the rhs address
         */
        bool operator==( const address& rhs ) const;

        /**
         * @brief returns false, if this address is the same as the rhs address
         */
        bool operator!=( const address& rhs ) const;

        /**
         * @brief random access iterator
         */
        typedef std::uint8_t const * const_iterator;

        /**
         * @brief returns an iterator to the first byte (LSB) of the address
         */
        const_iterator begin() const;

        /**
         * @brief returns an iterator one behind the last byte of the address
         */
        const_iterator end() const;
    private:
        static constexpr std::size_t address_size_in_bytes = 6;
        std::uint8_t value_[ address_size_in_bytes ];

        // this type is not intended for polymorphic use; not implemented
        void operator delete  ( void* ptr );
        void operator delete[]( void* ptr );
    };

    /**
     * @brief prints the given address in a human readable manner
     */
    std::ostream& operator<<( std::ostream& out, const address& a );

    /**
     * @brief data type containing a device address and the address type
     *        (public or random).
     *
     * A device address can either be a public or a random device address. To
     * construct one or the othere, use the subtypes public_device_address and
     * random_device_address.
     */
    class device_address : public address
    {
    public:
        device_address() = default;

        /**
         * @brief returns true, if this device address is a random device address.
         */
        bool is_random() const
        {
            return is_random_;
        }

        /**
         * @brief shortcut for !is_random()
         */
        bool is_public() const
        {
            return !is_random_;
        }

        using address::operator==;
        using address::operator!=;

        /**
         * @brief returns true, if this device address is the same as the rhs device address
         */
        bool operator==( const device_address& rhs ) const;

        /**
         * @brief returns false, if this device address is the same as the rhs device address
         */
        bool operator!=( const device_address& rhs ) const;

        device_address( const std::initializer_list< std::uint8_t >& initial_values, bool is_random )
            : address( initial_values )
            , is_random_( is_random )
        {}

        device_address( const std::uint8_t* initial_values, bool is_random )
            : address( initial_values )
            , is_random_( is_random )
        {}

    protected:
        explicit device_address( bool is_random )
            : address()
            , is_random_( is_random )
        {}

    private:
        bool is_random_;
    };

    /**
     * @brief data type containing a public device address
     *
     * The type is mainly ment to be a factory to construct a device_address
     */
    class public_device_address : public device_address
    {
    public:
        public_device_address() : device_address( false ) {}

        explicit public_device_address( const std::initializer_list< std::uint8_t >& initial_values )
            : device_address( initial_values, false ) {}

        /**
         * @brief initializing an address by taking 6 bytes from the given start of an array
         */
        explicit public_device_address( const std::uint8_t* initial_values )
            : device_address( initial_values, false ) {}
    };

    /**
     * @brief data type containing a random device address
     *
     * The type is mainly ment to be a factory to construct a device_address
     */
    class random_device_address : public device_address
    {
    public:
        random_device_address() : device_address( true ) {}

        explicit random_device_address( const std::initializer_list< std::uint8_t >& initial_values )
            : device_address( initial_values, true ) {}

        /**
         * @brief initializing an address by taking 6 bytes from the given start of an array
         */
        explicit random_device_address( const std::uint8_t* initial_values )
            : device_address( initial_values, true ) {}
    };

}

}

#endif
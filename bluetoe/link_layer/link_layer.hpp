#ifndef BLUETOE_LINK_LAYER_LINK_LAYER_HPP
#define BLUETOE_LINK_LAYER_LINK_LAYER_HPP

#include <bluetoe/link_layer/buffer.hpp>
#include <bluetoe/link_layer/delta_time.hpp>
#include <bluetoe/link_layer/options.hpp>
#include <bluetoe/link_layer/address.hpp>
#include <bluetoe/options.hpp>

namespace bluetoe {
namespace link_layer {


    /**
     * @brief link layer implementation
     *
     * Implements a binding to a server by implementing a link layout on top of a ScheduleRadio device.
     *
     * Missing Parameters:
     * - Advertising Interval
     * - opt. Address
     * - opt. Advertising Event type
     * - opt. Used Channels
     */
    template < class Server, template < class CallBack > class ScheduledRadio, typename ... Options >
    class link_layer : public ScheduledRadio< link_layer< Server, ScheduledRadio, Options... > >
    {
    public:
        link_layer();

        void run( Server& );

        void received( const read_buffer& receive );

        void timeout();
    private:
        // calculates the time point for the next advertising event
        delta_time next_adv_event();

        void fill_advertising_buffer( const Server& server );

        static constexpr std::size_t    max_advertising_data_size   = 31;
        static constexpr std::size_t    advertising_pdu_header_size = 2;
        static constexpr std::size_t    address_length              = 6;

        static constexpr unsigned       first_advertising_channel   = 37;
        static constexpr unsigned       last_advertising_channel    = 39;
        static constexpr unsigned       max_adv_perturbation_       = 10;

        static constexpr std::uint8_t   adv_ind_pdu_type_code       = 0;
        static constexpr std::uint8_t   header_txaddr_field         = 0x40;

        unsigned                        current_advertising_channel_;
        std::uint8_t                    adv_buffer_[ max_advertising_data_size + address_length + advertising_pdu_header_size ];
        std::size_t                     adv_size_;
        unsigned                        adv_perturbation_;
        const address                   address_;
        bool                            initial_advertising_;

        typedef                         advertising_interval< 100 > default_advertising_interval;
        typedef                         random_static_address       default_device_address;

        typedef typename ::bluetoe::details::find_by_meta_type<
            details::device_address_meta_type,
            Options..., default_device_address >::type              device_address;
    };

    // implementation
    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    link_layer< Server, ScheduledRadio, Options... >::link_layer()
        : current_advertising_channel_( first_advertising_channel )
        , adv_perturbation_( 0 )
        , address_( device_address::address( *this ) )
        , initial_advertising_( true )
    {
    }

    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    void link_layer< Server, ScheduledRadio, Options... >::run( Server& server )
    {
        // after the initial scheduling, the timeout and receive callback will setup the next scheduling
        if ( initial_advertising_ )
        {
            initial_advertising_ = false;
            fill_advertising_buffer( server );

            this->schedule_transmit_and_receive(
                current_advertising_channel_,
                write_buffer{ adv_buffer_, adv_size_ }, delta_time::now(),
                read_buffer() );
        }

        ScheduledRadio< link_layer< Server, ScheduledRadio, Options... > >::run();
    }

    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    void link_layer< Server, ScheduledRadio, Options... >::received( const read_buffer& receive )
    {
    }

    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    void link_layer< Server, ScheduledRadio, Options... >::timeout()
    {
        current_advertising_channel_ = current_advertising_channel_ == last_advertising_channel
            ? first_advertising_channel
            : current_advertising_channel_ + 1;

        const delta_time next_time = current_advertising_channel_ == first_advertising_channel
            ? next_adv_event()
            : delta_time::now();

        this->schedule_transmit_and_receive(
            current_advertising_channel_,
            write_buffer{ adv_buffer_, adv_size_ }, next_time,
            read_buffer() );
    }

    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    delta_time link_layer< Server, ScheduledRadio, Options... >::next_adv_event()
    {
        adv_perturbation_ = ( adv_perturbation_ + 7 ) % ( max_adv_perturbation_ + 1 );

        typedef typename ::bluetoe::details::find_by_meta_type<
            details::advertising_interval_meta_type,
            Options..., default_advertising_interval >::type adv_interval;

        return adv_interval::interval() + delta_time::msec( adv_perturbation_ );
    }

    template < class Server, template < class > class ScheduledRadio, typename ... Options >
    void link_layer< Server, ScheduledRadio, Options... >::fill_advertising_buffer( const Server& server )
    {
        adv_buffer_[ 0 ] = adv_ind_pdu_type_code;

        if ( device_address::is_random() )
            adv_buffer_[ 0 ] |= header_txaddr_field;

        adv_buffer_[ 1 ] = address_length + server.advertising_data( &adv_buffer_[ advertising_pdu_header_size + address_length ], max_advertising_data_size );
        std::copy( address_.begin(), address_.end(), &adv_buffer_[ 2 ] );
        adv_size_ = advertising_pdu_header_size + adv_buffer_[ 1 ];
    }
}
}

#endif

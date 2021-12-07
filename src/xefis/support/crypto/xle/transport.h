/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 *
 * XLE stands for Xefis Lossy Encryption
 */

#ifndef XEFIS__SUPPORT__CRYPTO__XLE__TRANSPORT_H__INCLUDED
#define XEFIS__SUPPORT__CRYPTO__XLE__TRANSPORT_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/crypto/hash.h>
#include <neutrino/exception.h>

// Xefis:
#include <xefis/config/all.h>


namespace xf::crypto::xle {

/**
 * Tool for packets encryption. Allows packets to be undelivered.
 */
class Transport
{
  public:
	using SequenceNumber = uint64_t;

	class DecryptionFailure: public Exception
	{
	  public:
		using Exception::Exception;
	};

  protected:
	static constexpr Hash::Algorithm const kHashAlgorithm = Hash::SHA3_256;

  public:
	// Ctor
	explicit
	Transport (BlobView ephemeral_session_key, size_t hmac_size = 12, BlobView salt = {}, BlobView hkdf_user_info = {});

  protected:
	size_t			_hmac_size;
	Blob			_hmac_key;
	Blob			_data_encryption_key;
	Blob			_seq_num_encryption_key;
	SequenceNumber	_sequence_number	{ 0 };
};


class Transmitter: public Transport
{
  public:
	// Ctor
	using Transport::Transport;

	/**
	 * Return next encrypted packet.
	 */
	Blob
	encrypt_packet (BlobView);
};


class Receiver: public Transport
{
  public:
	// Ctor
	using Transport::Transport;

	/**
	 * Return next decrypted packet.
	 */
	Blob
	decrypt_packet (BlobView data, std::optional<SequenceNumber> maximum_allowed_sequence_number = {});
};

} // namespace xf::crypto::xle

#endif


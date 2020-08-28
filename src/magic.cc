#include "magic.hh"

using namespace chess;

template <PieceType> Bitboard sliding_attacks(const Square, const Bitboard);

template <> Bitboard sliding_attacks<PieceType::Bishop>(const Square sq, const Bitboard occ)
{
	return ray_attacks<NorthEast, SouthEast, SouthWest, NorthWest>(square_bb(sq), occ);
}

template <> Bitboard sliding_attacks<PieceType::Rook>(const Square sq, const Bitboard occ)
{
	return ray_attacks<North, East, South, West>(square_bb(sq), occ);
}

#if defined(USE_FANCY)
template <PieceType T> constexpr util::array_t<Bitboard, Squares> PrecomputedMagics = {};

template <> constexpr util::array_t<Bitboard, Squares> PrecomputedMagics<PieceType::Bishop> =
{
	0x04408a8084008180, 0x5c20220a02023410, 0x9004010202080000, 0x0020a90100440021,
	0x2002021000400412, 0x900a022220004014, 0x006084886030a000, 0x6900602216104000,
	0x1500100238890400, 0x0430a08182060040, 0x0100104102002010, 0x2084040404814004,
	0x0080040420040020, 0x8000010420051000, 0x60404e0111084000, 0x0102010118010400,
	0x2010910a10010810, 0x0009000401080a01, 0x020a041002220060, 0x0006880802004100,
	0x080c100202021800, 0x1802012108021a00, 0x000281620a101310, 0x2810400080480800,
	0x8c1008014108410c, 0x0014056020210400, 0x0a40880c10004214, 0x4084080100202040,
	0x80250040cc044000, 0x1008490222008200, 0x060a0400008c0110, 0x2a01002001028861,
	0x400220a001100301, 0x910110480002d800, 0x0002081200040c10, 0x0090220080080480,
	0x0210020010020104, 0x4404410e00004800, 0x4001640400210100, 0x0024410440320050,
	0x0009012011102000, 0x5006012420000204, 0x0031101804002800, 0x1000806081200800,
	0x815106200a022500, 0xa010604080200500, 0x0605100411000041, 0x0148020a81340208,
	0x800d1402a0042010, 0x0802011088940344, 0x0180011088040000, 0x0814380084040088,
	0x00004a0963040097, 0x0464220e6a420081, 0x0020610102188000, 0x0204c10801010008,
	0x0620240108080220, 0x0004008a08210404, 0x200c001302949000, 0x2a00044080208804,
	0x0040048a91820210, 0x00100c2005012200, 0x1420900310040884, 0x805220940900c100
};

template <> constexpr util::array_t<Bitboard, Squares> PrecomputedMagics<PieceType::Rook> =
{
	0x0080001084284000, 0x2140022000100040, 0x0d00082001004010, 0x0900100100882084,
	0x0080080080040002, 0xc1800600010c0080, 0x0600040100a80200, 0x4100020088482900,
	0x1089800040008020, 0x4000404010002000, 0x4030808020001000, 0x0000801000080080,
	0x0006002004483200, 0x0021000803000400, 0x0221000200048100, 0x0001001190470002,
	0x0008888000400030, 0x10c0002020081000, 0x0250420018208200, 0x00800a0010422200,
	0x9001050011014800, 0x0020808002000400, 0x0808440010c80102, 0x0586020020410084,
	0x2080005040002000, 0x2020008101004000, 0x0420008080100020, 0x0008001010020100,
	0x2400080080800400, 0x0b62040080800200, 0x0086000200040108, 0x080408a200005104,
	0x1000400028800080, 0x2c01c02009401000, 0x0048882000801000, 0x400090000b002100,
	0x2214100801000500, 0x0000800400800200, 0x1828420124003088, 0x000035144a000084,
	0x0000400080208000, 0x8010044320044004, 0x0100200011010040, 0x0802002008420010,
	0x418c000800110100, 0x0002002010040400, 0x0002502841040022, 0x010040c08402001f,
	0x02204000800c2880, 0x0c10002000400040, 0x4500201200804200, 0x0e50010822910100,
	0x1200040008008080, 0x080e000400028080, 0x0000018208100400, 0x0440004400a10200,
	0xd308204011020082, 0xc447102040090181, 0x00060020420a8012, 0x00402e40180e00e2,
	0x1000054800110095, 0x0241000400020801, 0x0001300100880244, 0x0061122401004a86
};
#endif

template <PieceType T>
void MagicTable<T>::init()
{
	Bitboard edges, occ;
	std::size_t size = 0;

	for (Square sq = Square::A1; sq <= Square::H8; ++sq)
	{
		edges = ((Rank1BB | Rank8BB) & ~rank_bb(sq)) | ((FileABB | FileHBB) & ~file_bb(sq));

		auto &m = info(sq);
#if defined(USE_PDEP)
		m.postmask = sliding_attacks<T>(sq, 0);
		m.mask = m.postmask & ~edges;
#else
		m.mask = sliding_attacks<T>(sq, 0) & ~edges;
#endif

#if defined(USE_FANCY)
		m.magic = PrecomputedMagics<T>[util::underlying_value(sq)];
		m.shift = 64u - util::popcount_64(m.mask);
#endif

		m.attacks = sq == Square::A1 ? &_attacks[0] : info(sq - 1).attacks + size;

		occ  = 0;
		size = 0;
		do
		{
#if defined(USE_PDEP)
			m.attacks[index(sq, occ)] = util::pext_64(sliding_attacks<T>(sq, occ), m.postmask);
#else
			m.attacks[index(sq, occ)] = sliding_attacks<T>(sq, occ);
#endif

			++size;
			occ = (occ - m.mask) & m.mask;
		} while (occ);
	}
}

namespace chess::magics
{
#if !defined(USE_KOGGE_STONE)
	MagicTable<PieceType::Bishop> bishop_magics;
	MagicTable<PieceType::Rook> rook_magics;
#endif

	void init()
	{
#if !defined(USE_KOGGE_STONE)
		bishop_magics.init();
		rook_magics.init();
#endif
	}
}
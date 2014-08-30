/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef boden_wege_weg_h
#define boden_wege_weg_h

#include "../../display/simimg.h"
#include "../../simtypes.h"
#include "../../simobj.h"
#include "../../besch/weg_besch.h"
#include "../../dataobj/koord3d.h"


class karte_t;
class weg_besch_t;
class cbuffer_t;
class spieler_t;
template <class T> class slist_tpl;


// maximum number of months to store information
#define MAX_WAY_STAT_MONTHS 2

// number of different statistics collected
#define MAX_WAY_STATISTICS 2

enum way_statistics {
	WAY_STAT_GOODS   = 0, ///< number of goods transported over this weg
	WAY_STAT_CONVOIS = 1  ///< number of convois that passed this weg
};


/**
 * <p>Der Weg ist die Basisklasse fuer alle Verkehrswege in Simutrans.
 * Wege "geh�ren" immer zu einem Grund. Sie besitzen Richtungsbits sowie
 * eine Maske fuer Richtungsbits.</p>
 *
 * <p>Ein Weg geh�rt immer zu genau einer Wegsorte</p>
 *
 * <p>Kreuzungen werden dadurch unterst�tzt, da� ein Grund zwei Wege
 * enthalten kann (prinzipiell auch mehrere m�glich.</p>
 *
 * <p>Wegtyp -1 ist reserviert und kann nicht f�r Wege benutzt werden<p>
 *
 * @author Hj. Malthaner
 */
class weg_t : public obj_no_info_t
{
public:
	/**
	* Get list of all ways
	* @author Hj. Malthaner
	*/
	static const slist_tpl <weg_t *> & get_alle_wege();

	enum {
		HAS_SIDEWALK   = 0x01,
		IS_ELECTRIFIED = 0x02,
		HAS_SIGN       = 0x04,
		HAS_SIGNAL     = 0x08,
		HAS_WAYOBJ     = 0x10,
		HAS_CROSSING   = 0x20,
		IS_DIAGONAL    = 0x40, // marker for diagonal image
		IS_SNOW = 0x80	// marker, if above snowline currently
	};

	// see also unused: weg_besch_t::<anonym> enum { elevated=1, joined=7 /* only tram */, special=255 };
	enum system_type {
		type_flat     = 0,	///< flat track
		type_elevated = 1,	///< flag for elevated ways
		type_tram     = 7,	///< tram track (waytype = track_wt), hardcoded values everywhere ...
		type_underground = 64, ///< underground
		type_all      = 255
	};

private:
	/**
	* array for statistical values
	* MAX_WAY_STAT_MONTHS: [0] = actual value; [1] = last month value
	* MAX_WAY_STATISTICS: see #define at top of file
	* @author hsiegeln
	*/
	sint16 statistics[MAX_WAY_STAT_MONTHS][MAX_WAY_STATISTICS];

	/**
	* Way type description
	* @author Hj. Malthaner
	*/
	const weg_besch_t * besch;

	/**
	* Richtungsbits f�r den Weg. Norden ist oben rechts auf dem Monitor.
	* 1=Nord, 2=Ost, 4=Sued, 8=West
	* @author Hj. Malthaner
	*/
	uint8 ribi:4;

	/**
	* Maske f�r Richtungsbits
	* @author Hj. Malthaner
	*/
	uint8 ribi_maske:4;

	/**
	* flags like walkway, electrification, road sings
	* @author Hj. Malthaner
	*/
	uint8 flags;

	/**
	* max speed; could not be taken for besch, since other object may modify the speed
	* @author Hj. Malthaner
	*/
	uint16 max_speed;

	/**
	* Likewise for weight
	* @author: jamespetts
	*/
	uint32 max_axle_load;

	image_id bild;
	image_id after_bild;

	/**
	* Initializes all member variables
	* @author Hj. Malthaner
	*/
	void init();

	/**
	* initializes statistic array
	* @author hsiegeln
	*/
	void init_statistics();

	/*
	 * Way constraints for, e.g., loading gauges, types of electrification, etc.
	 * @author: jamespetts (modified by Bernd Gabriel)
	 */
	way_constraints_of_way_t way_constraints;

	// BG, 24.02.2012 performance enhancement avoid virtual method call, use inlined get_waytype()
	waytype_t waytype;

	/*
	* If this flag is true, players may not delete this way even if it is unowned unless they
	* build a diversionary route. Makes the way usable by all players regardless of ownership
	* and access settings. Permits upgrades but not downgrades, and prohibits private road signs.
	* @author: jamespetts
	*/
	bool public_right_of_way; 

	uint16 creation_month_year;
	uint16 last_renewal_month_year;
	uint32 tonnes_since_last_renewal;

protected:

	enum image_type { image_flat, image_slope, image_diagonal, image_switch };

	/**
	 * initializes both front and back images
	 * switch images are set in schiene_t::reserve
	 */
	void set_images(image_type typ, uint8 ribi, bool snow, bool switch_nw=false);

public:
	inline weg_t(waytype_t waytype, loadsave_t*) : obj_no_info_t(obj_t::way), waytype(waytype) { init(); }
	inline weg_t(waytype_t waytype) : obj_no_info_t(obj_t::way), waytype(waytype) { init(); }

	virtual ~weg_t();

	/* seasonal image recalculation */
	bool check_season(const long /*month*/);

#ifdef MULTI_THREAD
	void lock_mutex();
	void unlock_mutex();
#endif

	/* actual image recalculation */
	void calc_bild();

	/**
	* Setzt die erlaubte H�chstgeschwindigkeit
	* @author Hj. Malthaner
	*/
	void set_max_speed(sint32 s) { max_speed = s; }

	void set_max_axle_load(uint32 w);

	// Resets constraints to their base values. Used when removing way objects.
	void reset_way_constraints() { way_constraints = besch->get_way_constraints(); }

	void clear_way_constraints() { way_constraints.set_permissive(0); way_constraints.set_prohibitive(0); }

	/* Way constraints: determines whether vehicles
	 * can travel on this way. This method decodes
	 * the byte into bool values. See here for
	 * information on bitwise operations: 
	 * http://www.cprogramming.com/tutorial/bitwise_operators.html
	 * @author: jamespetts
	 * */
	
	const way_constraints_of_way_t& get_way_constraints() const { return way_constraints; }
	void add_way_constraints(const way_constraints_of_way_t& value) { way_constraints.add(value); }

	/**
	* Ermittelt die erlaubte H�chstgeschwindigkeit
	* @author Hj. Malthaner
	*/
	sint32 get_max_speed() const { return max_speed; }

	uint32 get_max_axle_load() const { return max_axle_load; }

	/**
	* Setzt neue Beschreibung. Ersetzt alte H�chstgeschwindigkeit
	* mit wert aus Beschreibung.
	*
	* Sets a new description. Replaces old with maximum speed
	* worth of description.
	* @author Hj. Malthaner
	*/
	void set_besch(const weg_besch_t *b);
	const weg_besch_t *get_besch() const { return besch; }

	// returns a way with the matching type
	static weg_t *alloc(waytype_t wt);

	// returns a string with the "official name of the waytype"
	static const char *waytype_to_string(waytype_t wt);

	virtual void rdwr(loadsave_t *file);

	/**
	* Info-text f�r diesen Weg
	* @author Hj. Malthaner
	*/
	virtual void info(cbuffer_t & buf, bool is_bridge = false) const;

	/**
	 * @return NULL wenn OK, ansonsten eine Fehlermeldung
	 * @author Hj. Malthaner
	 */
	virtual const char *ist_entfernbar(const spieler_t *sp, bool allow_public = false);

	/**
	* Wegtyp zur�ckliefern
	*/
	waytype_t get_waytype() const { return waytype; }

	/**
	* 'Jedes Ding braucht einen Typ.'
	* @return Gibt den typ des Objekts zur�ck.
	* @author Hj. Malthaner
	*/
	//typ get_typ() const { return obj_t::way; }

	/**
	* Die Bezeichnung des Wegs
	* @author Hj. Malthaner
	*/
	const char *get_name() const { return besch->get_name(); }

	/**
	* Setzt neue Richtungsbits f�r einen Weg.
	*
	* Nachdem die ribis ge�ndert werden, ist das weg_bild des
	* zugeh�rigen Grundes falsch (Ein Aufruf von grund_t::calc_bild()
	* zur Reparatur mu� folgen).
	* @param ribi Richtungsbits
	*/
	void ribi_add(ribi_t::ribi ribi) { this->ribi |= (uint8)ribi;}

	/**
	* Entfernt Richtungsbits von einem Weg.
	*
	* Nachdem die ribis ge�ndert werden, ist das weg_bild des
	* zugeh�rigen Grundes falsch (Ein Aufruf von grund_t::calc_bild()
	* zur Reparatur mu� folgen).
	* @param ribi Richtungsbits
	*/
	void ribi_rem(ribi_t::ribi ribi) { this->ribi &= (uint8)~ribi;}

	/**
	* Setzt Richtungsbits f�r den Weg.
	*
	* Nachdem die ribis ge�ndert werden, ist das weg_bild des
	* zugeh�rigen Grundes falsch (Ein Aufruf von grund_t::calc_bild()
	* zur Reparatur mu� folgen).
	* @param ribi Richtungsbits
	*/
	void set_ribi(ribi_t::ribi ribi) { this->ribi = (uint8)ribi;}

	/**
	* Ermittelt die unmaskierten Richtungsbits f�r den Weg.
	*/
	ribi_t::ribi get_ribi_unmasked() const { return (ribi_t::ribi)ribi; }

	/**
	* Ermittelt die (maskierten) Richtungsbits f�r den Weg.
	*/
	ribi_t::ribi get_ribi() const { return (ribi_t::ribi)(ribi & ~ribi_maske); }

	/**
	* f�r Signale ist es notwendig, bestimmte Richtungsbits auszumaskieren
	* damit Fahrzeuge nicht "von hinten" �ber Ampeln fahren k�nnen.
	* @param ribi Richtungsbits
	*/
	void set_ribi_maske(ribi_t::ribi ribi) { ribi_maske = (uint8)ribi; }
	ribi_t::ribi get_ribi_maske() const { return (ribi_t::ribi)ribi_maske; }

	/**
	 * called during map rotation
	 * @author priss
	 */
	void rotate90();

	/**
	* book statistics - is called very often and therefore inline
	* @author hsiegeln
	*/
	void book(int amount, way_statistics type) { statistics[0][type] += amount; }

	/**
	* return statistics value
	* always returns last month's value
	* @author hsiegeln
	*/
	int get_statistics(int type) const { return statistics[1][type]; }

	/**
	* new month
	* @author hsiegeln
	*/
	void neuer_monat();

	void check_diagonal();

	void count_sign();

	/* flag query routines */
	void set_gehweg(const bool yesno) { flags = (yesno ? flags | HAS_SIDEWALK : flags & ~HAS_SIDEWALK); }
	inline bool hat_gehweg() const { return flags & HAS_SIDEWALK; }

	void set_electrify(bool janein) {janein ? flags |= IS_ELECTRIFIED : flags &= ~IS_ELECTRIFIED;}
	inline bool is_electrified() const {return flags&IS_ELECTRIFIED; }

	inline bool has_sign() const {return flags&HAS_SIGN; }
	inline bool has_signal() const {return flags&HAS_SIGNAL; }
	inline bool has_wayobj() const {return flags&HAS_WAYOBJ; }
	inline bool is_crossing() const {return flags&HAS_CROSSING; }
	inline bool is_diagonal() const {return flags&IS_DIAGONAL; }
	inline bool is_snow() const {return flags&IS_SNOW; }

	// this is needed during a change from crossing to tram track
	void clear_crossing() { flags &= ~HAS_CROSSING; }

	/**
	 * Clear the has-sign flag when roadsign or signal got deleted.
	 * As there is only one of signal or roadsign on the way we can safely clear both flags.
	 */
	void clear_sign_flag() { flags &= ~(HAS_SIGN | HAS_SIGNAL); }

	inline void set_bild( image_id b ) { bild = b; }
	image_id get_bild() const {return bild;}

	inline void set_after_bild( image_id b ) { after_bild = b; }
	image_id get_after_bild() const {return after_bild;}

	// correct maintainace
	void laden_abschliessen();

	// Should a city adopt this, if it is being built/upgrade by player sp?
	bool should_city_adopt_this(const spieler_t* sp);

	bool is_public_right_of_way() const { return public_right_of_way; }
	void set_public_right_of_way(bool arg=true) { public_right_of_way = arg; }

} GCC_PACKED;

#endif

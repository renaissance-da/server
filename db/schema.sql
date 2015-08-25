--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: accident_item; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE accident_item (
    char_id integer NOT NULL,
    item_id integer NOT NULL,
    qty smallint NOT NULL,
    dur integer NOT NULL,
    mod smallint NOT NULL,
    temp boolean DEFAULT false NOT NULL,
    id bigint NOT NULL
);


--
-- Name: accident_item_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE accident_item_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: accident_item_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE accident_item_id_seq OWNED BY accident_item.id;


--
-- Name: buffs; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE buffs (
    buffid integer NOT NULL,
    kind integer NOT NULL,
    duration integer NOT NULL,
    icon smallint NOT NULL,
    received character varying(127) NOT NULL,
    ended character varying(127) NOT NULL,
    param1 integer DEFAULT 0 NOT NULL,
    param2 integer DEFAULT 0 NOT NULL
);


--
-- Name: characters; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE characters (
    name character varying(12) NOT NULL,
    password bytea NOT NULL,
    salt bytea NOT NULL,
    id integer NOT NULL,
    mapid integer DEFAULT 3029 NOT NULL,
    x smallint DEFAULT 25 NOT NULL,
    y smallint DEFAULT 48 NOT NULL,
    hp integer DEFAULT 50 NOT NULL,
    mp integer DEFAULT 30 NOT NULL,
    maxhp integer DEFAULT 50 NOT NULL,
    maxmp integer DEFAULT 30 NOT NULL,
    strength smallint DEFAULT 3 NOT NULL,
    intelligence smallint DEFAULT 3 NOT NULL,
    wisdom smallint DEFAULT 3 NOT NULL,
    dexterity smallint DEFAULT 3 NOT NULL,
    constitution smallint DEFAULT 3 NOT NULL,
    attribute_flag smallint DEFAULT 0 NOT NULL,
    hairstyle smallint DEFAULT 0 NOT NULL,
    haircolor smallint DEFAULT 0 NOT NULL,
    statpoints smallint DEFAULT 0 NOT NULL,
    gender character(1) DEFAULT 'm'::bpchar NOT NULL,
    path smallint DEFAULT 0 NOT NULL,
    pathmask smallint DEFAULT 0 NOT NULL,
    level smallint DEFAULT 1 NOT NULL,
    ab smallint DEFAULT 0 NOT NULL,
    exp bigint DEFAULT 1 NOT NULL,
    ap bigint DEFAULT 0 NOT NULL,
    gold integer DEFAULT 0 NOT NULL,
    priv smallint DEFAULT 0 NOT NULL,
    dead smallint DEFAULT 0 NOT NULL,
    labor smallint DEFAULT 0 NOT NULL,
    laborreset integer DEFAULT 0 NOT NULL,
    nation smallint DEFAULT 1 NOT NULL,
    last_login integer DEFAULT 2147483647 NOT NULL,
    stored_gold integer DEFAULT 0 NOT NULL,
    accident_gold integer DEFAULT 0 NOT NULL,
    settings integer DEFAULT 221 NOT NULL,
    guild_rank smallint,
    guild_id integer,
    banned integer DEFAULT 0 NOT NULL,
    CONSTRAINT characters_stored_gold_check CHECK ((stored_gold >= 0))
);


--
-- Name: characters_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE characters_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: characters_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE characters_id_seq OWNED BY characters.id;


--
-- Name: consumable; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE consumable (
    item_id integer NOT NULL,
    type smallint NOT NULL,
    param integer DEFAULT 0 NOT NULL
);


--
-- Name: drops; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE drops (
    mob_id integer NOT NULL,
    item_id integer NOT NULL,
    rate smallint,
    mod smallint DEFAULT 0 NOT NULL,
    id integer NOT NULL,
    CONSTRAINT drops_rate_check CHECK (((rate > 0) AND (rate <= 1000)))
);


--
-- Name: drops_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE drops_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: drops_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE drops_id_seq OWNED BY drops.id;


--
-- Name: equip; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE equip (
    item_id integer NOT NULL,
    slot smallint NOT NULL,
    dur integer NOT NULL,
    gender character(1),
    equip_apr smallint DEFAULT 0 NOT NULL,
    underwear smallint DEFAULT 0 NOT NULL,
    level smallint DEFAULT 1 NOT NULL,
    path smallint DEFAULT 0 NOT NULL,
    wmin integer DEFAULT 0 NOT NULL,
    wmax integer DEFAULT 0 NOT NULL,
    ac smallint DEFAULT 0 NOT NULL,
    hp integer DEFAULT 0 NOT NULL,
    mp integer DEFAULT 0 NOT NULL,
    hit smallint DEFAULT 0 NOT NULL,
    dmg smallint DEFAULT 0 NOT NULL,
    mr smallint DEFAULT 0 NOT NULL,
    strength smallint DEFAULT 0 NOT NULL,
    con smallint DEFAULT 0 NOT NULL,
    dex smallint DEFAULT 0 NOT NULL,
    intelligence smallint DEFAULT 0 NOT NULL,
    wisdom smallint DEFAULT 0 NOT NULL,
    regen smallint DEFAULT 0 NOT NULL,
    improvable smallint DEFAULT 0 NOT NULL,
    ele smallint DEFAULT 0 NOT NULL,
    flags integer DEFAULT 0 NOT NULL,
    lmin integer DEFAULT 0 NOT NULL,
    lmax integer DEFAULT 0 NOT NULL,
    CONSTRAINT equip_path_check CHECK (((path >= 0) AND (path < 12))),
    CONSTRAINT equip_slot_check CHECK (((slot > 0) AND (slot < 19)))
);


--
-- Name: field; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE field (
    id integer NOT NULL,
    world_map smallint DEFAULT 1 NOT NULL,
    field_name character varying(127)
);


--
-- Name: field_dest; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE field_dest (
    mapid integer,
    px smallint NOT NULL,
    py smallint NOT NULL,
    xdest smallint NOT NULL,
    ydest smallint NOT NULL,
    name character varying(127) NOT NULL,
    id integer NOT NULL
);


--
-- Name: field_dest_on; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE field_dest_on (
    destid integer NOT NULL,
    fieldid integer NOT NULL,
    id integer NOT NULL
);


--
-- Name: field_dest_on_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE field_dest_on_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: field_dest_on_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE field_dest_on_id_seq OWNED BY field_dest_on.id;


--
-- Name: guild; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE guild (
    id integer NOT NULL,
    name text NOT NULL
);


--
-- Name: guild_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE guild_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: guild_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE guild_id_seq OWNED BY guild.id;


--
-- Name: has_effect; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE has_effect (
    char_id integer NOT NULL,
    buff_id integer NOT NULL,
    duration integer NOT NULL,
    id bigint NOT NULL
);


--
-- Name: has_effect_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE has_effect_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: has_effect_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE has_effect_id_seq OWNED BY has_effect.id;


--
-- Name: has_item; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE has_item (
    char_id integer NOT NULL,
    item_id integer NOT NULL,
    slot smallint NOT NULL,
    qty smallint,
    dur integer,
    mod smallint,
    identified smallint DEFAULT 0 NOT NULL,
    id bigint NOT NULL,
    CONSTRAINT has_item_slot_check CHECK (((slot >= 0) AND (slot < 78)))
);


--
-- Name: has_item_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE has_item_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: has_item_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE has_item_id_seq OWNED BY has_item.id;


--
-- Name: has_legend; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE has_legend (
    char_id integer NOT NULL,
    mark_id integer NOT NULL,
    text_param character varying(127) NOT NULL,
    int_param integer NOT NULL,
    "timestamp" integer NOT NULL,
    id bigint NOT NULL
);


--
-- Name: has_legend_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE has_legend_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: has_legend_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE has_legend_id_seq OWNED BY has_legend.id;


--
-- Name: has_secret; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE has_secret (
    char_id integer NOT NULL,
    skill_id integer NOT NULL,
    slot smallint NOT NULL,
    level smallint DEFAULT 0 NOT NULL,
    uses integer DEFAULT 0 NOT NULL,
    cd integer DEFAULT 0 NOT NULL,
    id bigint NOT NULL,
    CONSTRAINT has_secret_slot_check CHECK ((slot < 87))
);


--
-- Name: has_secret_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE has_secret_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: has_secret_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE has_secret_id_seq OWNED BY has_secret.id;


--
-- Name: has_skill; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE has_skill (
    char_id integer NOT NULL,
    skill_id integer NOT NULL,
    slot smallint NOT NULL,
    level smallint DEFAULT 0 NOT NULL,
    uses integer DEFAULT 0 NOT NULL,
    cd integer DEFAULT 0 NOT NULL,
    id bigint NOT NULL,
    CONSTRAINT has_skill_slot_check CHECK ((slot < 87))
);


--
-- Name: has_skill_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE has_skill_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: has_skill_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE has_skill_id_seq OWNED BY has_skill.id;


--
-- Name: ip_banlist; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE ip_banlist (
    addr integer NOT NULL,
    exp integer
);


--
-- Name: item; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE item (
    ground_apr smallint NOT NULL,
    weight smallint NOT NULL,
    id integer NOT NULL,
    name character varying(128) NOT NULL,
    value integer NOT NULL,
    id_name character varying(128),
    max_stack smallint,
    flags integer DEFAULT 0 NOT NULL
);


--
-- Name: legend_mark; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE legend_mark (
    id integer NOT NULL,
    prefix character varying(4) NOT NULL,
    text character varying(127) NOT NULL,
    param_fmt smallint DEFAULT 0 NOT NULL,
    icon smallint DEFAULT 0 NOT NULL,
    color smallint DEFAULT 16 NOT NULL
);


--
-- Name: map_db; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE map_db (
    mapid integer NOT NULL,
    name character varying(127) NOT NULL,
    width smallint NOT NULL,
    height smallint NOT NULL,
    bgm smallint,
    flags integer DEFAULT 0 NOT NULL
);


--
-- Name: mob; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE mob (
    level smallint NOT NULL,
    hplo integer NOT NULL,
    name character varying(20) NOT NULL,
    description character varying(128) NOT NULL,
    mindmg integer NOT NULL,
    maxdmg integer NOT NULL,
    exp integer NOT NULL,
    id integer NOT NULL,
    apr smallint NOT NULL,
    mode smallint NOT NULL,
    ac smallint NOT NULL,
    power smallint DEFAULT 0 NOT NULL,
    atk smallint DEFAULT 0 NOT NULL,
    defense smallint DEFAULT 0 NOT NULL,
    gold_min integer DEFAULT 0 NOT NULL,
    gold_max integer DEFAULT 0 NOT NULL,
    mr smallint NOT NULL,
    dex smallint NOT NULL,
    hpmax integer NOT NULL,
    size character(1) DEFAULT 's'::bpchar NOT NULL,
    regen smallint DEFAULT 0 NOT NULL,
    submode smallint NOT NULL
);


--
-- Name: mob_skill; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE mob_skill (
    mob_id integer NOT NULL,
    skill_id integer NOT NULL,
    rate integer NOT NULL,
    id integer NOT NULL
);


--
-- Name: mob_skill_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE mob_skill_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: mob_skill_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE mob_skill_id_seq OWNED BY mob_skill.id;


--
-- Name: portal; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE portal (
    mapid integer NOT NULL,
    x smallint NOT NULL,
    y smallint NOT NULL,
    destid integer NOT NULL,
    destx smallint NOT NULL,
    desty smallint NOT NULL,
    field_id integer,
    min_level smallint DEFAULT 0 NOT NULL,
    max_level smallint DEFAULT 0 NOT NULL,
    id integer NOT NULL
);


--
-- Name: portal_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE portal_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: portal_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE portal_id_seq OWNED BY portal.id;


--
-- Name: quest_progress; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE quest_progress (
    char_id integer NOT NULL,
    quest_id integer NOT NULL,
    quest_flags integer DEFAULT 0 NOT NULL,
    quest_timer integer DEFAULT 0 NOT NULL,
    id bigint NOT NULL
);


--
-- Name: quest_progress_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE quest_progress_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: quest_progress_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE quest_progress_id_seq OWNED BY quest_progress.id;


--
-- Name: secretinfo; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE secretinfo (
    sid integer NOT NULL,
    baselines smallint DEFAULT 0 NOT NULL,
    target smallint DEFAULT 0 NOT NULL,
    descfile character varying(127) NOT NULL,
    elem smallint NOT NULL
);


--
-- Name: sign; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE sign (
    mapid integer NOT NULL,
    x smallint NOT NULL,
    y smallint NOT NULL,
    signtext character varying(127) NOT NULL,
    id integer NOT NULL
);


--
-- Name: sign_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE sign_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: sign_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE sign_id_seq OWNED BY sign.id;


--
-- Name: skillinfo; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE skillinfo (
    skillid integer NOT NULL,
    name character varying(127) NOT NULL,
    max_warrior smallint NOT NULL,
    max_monk smallint NOT NULL,
    max_rogue smallint NOT NULL,
    max_priest smallint NOT NULL,
    max_wizard smallint NOT NULL,
    max_master smallint NOT NULL,
    anim smallint NOT NULL,
    anim_time smallint NOT NULL,
    sound smallint NOT NULL,
    cd integer NOT NULL,
    icon smallint NOT NULL,
    levelrate integer,
    effect smallint,
    effect_self smallint,
    target smallint NOT NULL,
    flags integer NOT NULL,
    buff integer,
    strreq smallint,
    dexreq smallint,
    conreq smallint,
    intreq smallint,
    wisreq smallint,
    levreq smallint,
    pathreq smallint,
    mpcost integer DEFAULT 0 NOT NULL,
    goldcost integer,
    item1 integer,
    item1amt integer,
    item2 integer,
    item2amt integer,
    item3 integer,
    item3amt integer,
    item1mod smallint,
    item2mod smallint,
    item3mod smallint,
    CONSTRAINT skillinfo_skillid_check CHECK ((skillid <> 0))
);


--
-- Name: skillreqskill; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE skillreqskill (
    skillid integer NOT NULL,
    reqid integer NOT NULL,
    levreq smallint,
    id integer NOT NULL,
    CONSTRAINT skillreqskill_levreq_check CHECK (((levreq > 0) AND (levreq <= 100)))
);


--
-- Name: skillreqskill_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE skillreqskill_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: skillreqskill_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE skillreqskill_id_seq OWNED BY skillreqskill.id;


--
-- Name: spawner; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE spawner (
    mob_id integer NOT NULL,
    map_id integer NOT NULL,
    qty smallint NOT NULL,
    frequency integer DEFAULT 180 NOT NULL,
    id integer NOT NULL
);


--
-- Name: spawner_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE spawner_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: spawner_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE spawner_id_seq OWNED BY spawner.id;


--
-- Name: storage; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE storage (
    char_id integer NOT NULL,
    item_id integer NOT NULL,
    qty integer NOT NULL,
    mod smallint NOT NULL,
    id bigint NOT NULL
);


--
-- Name: storage_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE storage_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: storage_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE storage_id_seq OWNED BY storage.id;


--
-- Name: trackers; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE trackers (
    char_id integer NOT NULL,
    quest_id integer NOT NULL,
    mob_id integer NOT NULL,
    qty integer NOT NULL,
    id bigint NOT NULL
);


--
-- Name: trackers_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE trackers_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: trackers_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE trackers_id_seq OWNED BY trackers.id;


--
-- Name: update_character; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE update_character (
    mapid integer,
    x smallint,
    y smallint,
    hp integer,
    mp integer,
    maxhp integer,
    maxmp integer,
    strength smallint,
    intelligence smallint,
    wisdom smallint,
    dexterity smallint,
    constitution smallint,
    statpoints smallint,
    level smallint,
    ab smallint,
    dead smallint,
    exp bigint,
    ap bigint,
    id integer,
    gold integer,
    labor smallint,
    laborreset integer,
    last_login integer,
    stored_gold integer,
    accident_gold integer,
    settings integer
);


--
-- Name: update_secret; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE update_secret (
    char_id integer,
    level smallint,
    slot smallint,
    uses integer,
    cd integer
);


--
-- Name: update_skill; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE update_skill (
    char_id integer,
    level smallint,
    slot smallint,
    uses integer,
    cd integer
);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY accident_item ALTER COLUMN id SET DEFAULT nextval('accident_item_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY characters ALTER COLUMN id SET DEFAULT nextval('characters_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY drops ALTER COLUMN id SET DEFAULT nextval('drops_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY field_dest_on ALTER COLUMN id SET DEFAULT nextval('field_dest_on_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY guild ALTER COLUMN id SET DEFAULT nextval('guild_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_effect ALTER COLUMN id SET DEFAULT nextval('has_effect_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_item ALTER COLUMN id SET DEFAULT nextval('has_item_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_legend ALTER COLUMN id SET DEFAULT nextval('has_legend_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_secret ALTER COLUMN id SET DEFAULT nextval('has_secret_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_skill ALTER COLUMN id SET DEFAULT nextval('has_skill_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY mob_skill ALTER COLUMN id SET DEFAULT nextval('mob_skill_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY portal ALTER COLUMN id SET DEFAULT nextval('portal_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY quest_progress ALTER COLUMN id SET DEFAULT nextval('quest_progress_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY sign ALTER COLUMN id SET DEFAULT nextval('sign_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillreqskill ALTER COLUMN id SET DEFAULT nextval('skillreqskill_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY spawner ALTER COLUMN id SET DEFAULT nextval('spawner_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY storage ALTER COLUMN id SET DEFAULT nextval('storage_id_seq'::regclass);


--
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY trackers ALTER COLUMN id SET DEFAULT nextval('trackers_id_seq'::regclass);


--
-- Name: accident_item_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY accident_item
    ADD CONSTRAINT accident_item_pkey PRIMARY KEY (id);


--
-- Name: buffs_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY buffs
    ADD CONSTRAINT buffs_pkey PRIMARY KEY (buffid);


--
-- Name: characters_name_key; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY characters
    ADD CONSTRAINT characters_name_key UNIQUE (name);


--
-- Name: characters_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY characters
    ADD CONSTRAINT characters_pkey PRIMARY KEY (id);


--
-- Name: consumable_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY consumable
    ADD CONSTRAINT consumable_pkey PRIMARY KEY (item_id);


--
-- Name: drop_once; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY drops
    ADD CONSTRAINT drop_once UNIQUE (mob_id, item_id);


--
-- Name: drops_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY drops
    ADD CONSTRAINT drops_pkey PRIMARY KEY (id);


--
-- Name: effect_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_effect
    ADD CONSTRAINT effect_unique UNIQUE (char_id, buff_id);


--
-- Name: equip_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY equip
    ADD CONSTRAINT equip_pkey PRIMARY KEY (item_id);


--
-- Name: field_dest_on_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY field_dest_on
    ADD CONSTRAINT field_dest_on_pkey PRIMARY KEY (id);


--
-- Name: field_dest_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY field_dest
    ADD CONSTRAINT field_dest_pkey PRIMARY KEY (id);


--
-- Name: field_dest_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY field_dest_on
    ADD CONSTRAINT field_dest_unique UNIQUE (destid, fieldid);


--
-- Name: field_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY field
    ADD CONSTRAINT field_pkey PRIMARY KEY (id);


--
-- Name: guild_name_key; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY guild
    ADD CONSTRAINT guild_name_key UNIQUE (name);


--
-- Name: guild_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY guild
    ADD CONSTRAINT guild_pkey PRIMARY KEY (id);


--
-- Name: has_effect_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_effect
    ADD CONSTRAINT has_effect_pkey PRIMARY KEY (id);


--
-- Name: has_item_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_item
    ADD CONSTRAINT has_item_pkey PRIMARY KEY (id);


--
-- Name: has_legend_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_legend
    ADD CONSTRAINT has_legend_pkey PRIMARY KEY (id);


--
-- Name: has_secret_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_secret
    ADD CONSTRAINT has_secret_pkey PRIMARY KEY (id);


--
-- Name: has_skill_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_skill
    ADD CONSTRAINT has_skill_pkey PRIMARY KEY (id);


--
-- Name: ip_banlist_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY ip_banlist
    ADD CONSTRAINT ip_banlist_pkey PRIMARY KEY (addr);


--
-- Name: item_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY item
    ADD CONSTRAINT item_pkey PRIMARY KEY (id);


--
-- Name: legend_mark_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY legend_mark
    ADD CONSTRAINT legend_mark_pkey PRIMARY KEY (id);


--
-- Name: legend_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_legend
    ADD CONSTRAINT legend_unique UNIQUE (char_id, mark_id);


--
-- Name: map_db_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY map_db
    ADD CONSTRAINT map_db_pkey PRIMARY KEY (mapid);


--
-- Name: mob_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY mob
    ADD CONSTRAINT mob_pkey PRIMARY KEY (id);


--
-- Name: mob_skill_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY mob_skill
    ADD CONSTRAINT mob_skill_pkey PRIMARY KEY (id);


--
-- Name: mob_skill_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY mob_skill
    ADD CONSTRAINT mob_skill_unique UNIQUE (mob_id, skill_id);


--
-- Name: portal_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY portal
    ADD CONSTRAINT portal_pkey PRIMARY KEY (id);


--
-- Name: portal_tile_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY portal
    ADD CONSTRAINT portal_tile_unique UNIQUE (mapid, x, y);


--
-- Name: quest_progress_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY quest_progress
    ADD CONSTRAINT quest_progress_pkey PRIMARY KEY (id);


--
-- Name: quest_progress_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY quest_progress
    ADD CONSTRAINT quest_progress_unique UNIQUE (char_id, quest_id);


--
-- Name: secret_slot_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_secret
    ADD CONSTRAINT secret_slot_unique UNIQUE (char_id, slot) DEFERRABLE INITIALLY DEFERRED;


--
-- Name: secretinfo_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY secretinfo
    ADD CONSTRAINT secretinfo_pkey PRIMARY KEY (sid);


--
-- Name: secrets_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_secret
    ADD CONSTRAINT secrets_unique UNIQUE (char_id, skill_id);


--
-- Name: sign_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY sign
    ADD CONSTRAINT sign_pkey PRIMARY KEY (id);


--
-- Name: sign_tile_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY sign
    ADD CONSTRAINT sign_tile_unique UNIQUE (mapid, x, y);


--
-- Name: skill_req_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skillreqskill
    ADD CONSTRAINT skill_req_unique UNIQUE (skillid, reqid);


--
-- Name: skill_slot_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_skill
    ADD CONSTRAINT skill_slot_unique UNIQUE (char_id, slot) DEFERRABLE INITIALLY DEFERRED;


--
-- Name: skillinfo_name_key; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_name_key UNIQUE (name);


--
-- Name: skillinfo_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_pkey PRIMARY KEY (skillid);


--
-- Name: skillreqskill_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skillreqskill
    ADD CONSTRAINT skillreqskill_pkey PRIMARY KEY (id);


--
-- Name: skills_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_skill
    ADD CONSTRAINT skills_unique UNIQUE (char_id, skill_id);


--
-- Name: slot_unique; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY has_item
    ADD CONSTRAINT slot_unique UNIQUE (char_id, slot);


--
-- Name: spawner_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY spawner
    ADD CONSTRAINT spawner_pkey PRIMARY KEY (id);


--
-- Name: storage_char_id_item_id_mod_key; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY storage
    ADD CONSTRAINT storage_char_id_item_id_mod_key UNIQUE (char_id, item_id, mod);


--
-- Name: storage_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY storage
    ADD CONSTRAINT storage_pkey PRIMARY KEY (id);


--
-- Name: trackers_pkey; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY trackers
    ADD CONSTRAINT trackers_pkey PRIMARY KEY (id);


--
-- Name: accident_item_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY accident_item
    ADD CONSTRAINT accident_item_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: accident_item_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY accident_item
    ADD CONSTRAINT accident_item_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: characters_guild_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY characters
    ADD CONSTRAINT characters_guild_id_fkey FOREIGN KEY (guild_id) REFERENCES guild(id) ON DELETE SET NULL;


--
-- Name: consumable_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY consumable
    ADD CONSTRAINT consumable_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: drops_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY drops
    ADD CONSTRAINT drops_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: drops_mob_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY drops
    ADD CONSTRAINT drops_mob_id_fkey FOREIGN KEY (mob_id) REFERENCES mob(id);


--
-- Name: equip_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY equip
    ADD CONSTRAINT equip_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: field_dest_mapid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY field_dest
    ADD CONSTRAINT field_dest_mapid_fkey FOREIGN KEY (mapid) REFERENCES map_db(mapid);


--
-- Name: field_dest_on_destid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY field_dest_on
    ADD CONSTRAINT field_dest_on_destid_fkey FOREIGN KEY (destid) REFERENCES field_dest(id);


--
-- Name: field_dest_on_fieldid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY field_dest_on
    ADD CONSTRAINT field_dest_on_fieldid_fkey FOREIGN KEY (fieldid) REFERENCES field(id);


--
-- Name: has_effect_buff_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_effect
    ADD CONSTRAINT has_effect_buff_id_fkey FOREIGN KEY (buff_id) REFERENCES buffs(buffid);


--
-- Name: has_effect_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_effect
    ADD CONSTRAINT has_effect_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: has_item_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_item
    ADD CONSTRAINT has_item_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: has_item_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_item
    ADD CONSTRAINT has_item_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: has_legend_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_legend
    ADD CONSTRAINT has_legend_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: has_legend_mark_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_legend
    ADD CONSTRAINT has_legend_mark_id_fkey FOREIGN KEY (mark_id) REFERENCES legend_mark(id);


--
-- Name: has_secret_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_secret
    ADD CONSTRAINT has_secret_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: has_secret_skill_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_secret
    ADD CONSTRAINT has_secret_skill_id_fkey FOREIGN KEY (skill_id) REFERENCES secretinfo(sid);


--
-- Name: has_skill_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_skill
    ADD CONSTRAINT has_skill_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: has_skill_skill_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY has_skill
    ADD CONSTRAINT has_skill_skill_id_fkey FOREIGN KEY (skill_id) REFERENCES skillinfo(skillid);


--
-- Name: mob_skill_mob_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY mob_skill
    ADD CONSTRAINT mob_skill_mob_id_fkey FOREIGN KEY (mob_id) REFERENCES mob(id);


--
-- Name: mob_skill_skill_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY mob_skill
    ADD CONSTRAINT mob_skill_skill_id_fkey FOREIGN KEY (skill_id) REFERENCES skillinfo(skillid);


--
-- Name: portal_destid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY portal
    ADD CONSTRAINT portal_destid_fkey FOREIGN KEY (destid) REFERENCES map_db(mapid);


--
-- Name: portal_field_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY portal
    ADD CONSTRAINT portal_field_id_fkey FOREIGN KEY (field_id) REFERENCES field(id);


--
-- Name: portal_mapid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY portal
    ADD CONSTRAINT portal_mapid_fkey FOREIGN KEY (mapid) REFERENCES map_db(mapid);


--
-- Name: quest_progress_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY quest_progress
    ADD CONSTRAINT quest_progress_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: secretinfo_sid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY secretinfo
    ADD CONSTRAINT secretinfo_sid_fkey FOREIGN KEY (sid) REFERENCES skillinfo(skillid);


--
-- Name: sign_mapid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY sign
    ADD CONSTRAINT sign_mapid_fkey FOREIGN KEY (mapid) REFERENCES map_db(mapid);


--
-- Name: skillinfo_buff_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_buff_fkey FOREIGN KEY (buff) REFERENCES buffs(buffid);


--
-- Name: skillinfo_item1_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_item1_fkey FOREIGN KEY (item1) REFERENCES item(id);


--
-- Name: skillinfo_item2_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_item2_fkey FOREIGN KEY (item2) REFERENCES item(id);


--
-- Name: skillinfo_item3_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillinfo
    ADD CONSTRAINT skillinfo_item3_fkey FOREIGN KEY (item3) REFERENCES item(id);


--
-- Name: skillreqskill_reqid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillreqskill
    ADD CONSTRAINT skillreqskill_reqid_fkey FOREIGN KEY (reqid) REFERENCES skillinfo(skillid);


--
-- Name: skillreqskill_skillid_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY skillreqskill
    ADD CONSTRAINT skillreqskill_skillid_fkey FOREIGN KEY (skillid) REFERENCES skillinfo(skillid);


--
-- Name: spawner_map_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spawner
    ADD CONSTRAINT spawner_map_id_fkey FOREIGN KEY (map_id) REFERENCES map_db(mapid);


--
-- Name: spawner_mob_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY spawner
    ADD CONSTRAINT spawner_mob_id_fkey FOREIGN KEY (mob_id) REFERENCES mob(id);


--
-- Name: storage_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY storage
    ADD CONSTRAINT storage_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: storage_item_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY storage
    ADD CONSTRAINT storage_item_id_fkey FOREIGN KEY (item_id) REFERENCES item(id);


--
-- Name: trackers_char_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY trackers
    ADD CONSTRAINT trackers_char_id_fkey FOREIGN KEY (char_id) REFERENCES characters(id) ON DELETE CASCADE;


--
-- Name: public; Type: ACL; Schema: -; Owner: -
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--


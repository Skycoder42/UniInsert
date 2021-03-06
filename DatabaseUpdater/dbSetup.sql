CREATE TABLE Meta (
	UnicodeVersion	TEXT NOT NULL
);
CREATE TABLE Symbols (
	Code	INTEGER NOT NULL UNIQUE,
	Name	TEXT,
	PRIMARY KEY(Code)
);
CREATE TABLE Recent (
	Code	INTEGER NOT NULL UNIQUE,
	SymCount	INTEGER NOT NULL DEFAULT 0,
	PRIMARY KEY(Code),
	FOREIGN KEY(Code) REFERENCES Symbols(Code)
);
CREATE TABLE EmojiMapping (
	GroupID	INTEGER NOT NULL,
	EmojiID	INTEGER NOT NULL,
	SortHint	INTEGER NOT NULL,
	PRIMARY KEY(GroupID, EmojiID)
);
CREATE TABLE EmojiGroups (
	ID	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	Name	TEXT NOT NULL,
	SortHint	INTEGER
);
CREATE TABLE Blocks (
	ID	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	Name	TEXT NOT NULL,
	BlockStart	INTEGER UNIQUE,
	BlockEnd	INTEGER CHECK(BlockEnd >= BlockStart) UNIQUE
);
CREATE TABLE Aliases (
	Code	INTEGER NOT NULL,
	NameAlias	TEXT NOT NULL,
	UNIQUE(Code, NameAlias),
	FOREIGN KEY(Code) REFERENCES Symbols(Code)
);
CREATE INDEX TopRecent ON Recent(SymCount DESC);
CREATE INDEX SymbolSearch ON Aliases(NameAlias);
CREATE INDEX MappingSort ON EmojiMapping(SortHint ASC);
CREATE INDEX GroupSorting ON EmojiGroups(SortHint ASC);
CREATE INDEX BlockSearch ON Blocks(Name);

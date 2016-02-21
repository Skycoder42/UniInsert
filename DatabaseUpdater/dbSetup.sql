CREATE TABLE 'Meta' (
	'UnicodeVersion'	TEXT NOT NULL
);
CREATE TABLE 'Symbols' (
	'Code'	INTEGER NOT NULL UNIQUE,
	'Name'	TEXT,
	PRIMARY KEY('Code')
);
CREATE TABLE 'Recent' (
	'Code'	INTEGER NOT NULL UNIQUE,
	'Count'	INTEGER NOT NULL DEFAULT 0,
	PRIMARY KEY('Code'),
	FOREIGN KEY('Code') REFERENCES 'Symbols'('Code')
);
CREATE TABLE 'EmojiMapping' (
	'GroupID'	INTEGER NOT NULL,
	'EmojiID'	INTEGER NOT NULL,
	'SortHint'	INTEGER NOT NULL,
	PRIMARY KEY('GroupID', 'EmojiID')
);
CREATE TABLE 'EmojiGroups' (
	'ID'	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	'Name'	TEXT NOT NULL,
	'SortHint'	INTEGER
);
CREATE TABLE 'Blocks' (
	'ID'	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	'Name'	TEXT NOT NULL,
	'Start'	INTEGER UNIQUE,
	'End'	INTEGER CHECK('End' >= 'Start') UNIQUE
);
CREATE TABLE 'Aliases' (
	'Code'	INTEGER NOT NULL,
	'Alias'	TEXT NOT NULL,
	FOREIGN KEY('Code') REFERENCES Symbols('Code')
);
CREATE INDEX 'TopRecent' ON 'Recent'('Count' DESC);
CREATE INDEX 'SymbolSearch' ON 'Aliases'('Alias');
CREATE INDEX 'MappingSort' ON 'EmojiMapping'('SortHint' ASC);
CREATE INDEX 'GroupSorting' ON 'EmojiGroups'('SortHint' ASC);
CREATE INDEX 'BlockSearch' ON 'Blocks'('Name');

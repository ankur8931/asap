create proc my_getindexes @objname nvarchar(776)
-- based on sp_helpindex
-- created: lfcastro 03feb03
-- modified: lfcastro 05feb03
as
	set nocount on

	declare	@objid int, 		-- the object id of the table
		@indid smallint,		-- the index id of an index
		@groupid smallint,	-- the filegroup id of an index
		@indname sysname,	-- name of an index
		@groupname sysname,
		@status int,
		@keys nvarchar(2126),
		@dbname sysname,
		@olddbname sysname

	-- change database, if necessary
	select @dbname = parsename(@objname,3)

--	if @dbname is not null and @dbname <> db_name()
--	begin
--		select @olddbname = db_name()	
--		use @dbname
--	end

	-- does the table exist?
	select @objid = object_id(@objname)
	
	if @objid is NULL
	begin
		select @dbname = db_name()
--		use @olddbname
		raiserror(15009,-1,-1,@objname,@dbname)
		return(1)
	end

	-- open cursor over indexes
	declare	ind_cursor cursor local static for
		select 		indid, groupid, name, status 
		from 		sysindexes
		where 		id = @objid and indid > 0 and indid < 255
				and (status & 64) = 0
		order by	indid

	open ind_cursor
	fetch ind_cursor into @indid, @groupid, @indname, @status

	-- if no index, quit
	if @@fetch_status < 0
	begin
		deallocate ind_cursor
--		use @olddbname
--		raiserror(15472,-1,-1)
		return(0)
	end

	-- create temp table with index names
	create table #indtab
	(
		index_name	sysname collate database_default NOT NULL,
		stats		int,
		groupname	sysname collate database_default NOT NULL,
		index_keys	nvarchar(2126) collate database_default NOT NULL
	)

	-- collect keys for each index into temporary table
	while @@fetch_status >= 0
	begin
		-- figure out what the keys are
		declare @i int, @thiskey nvarchar(131)

		select @keys = index_col(@objname, @indid, 1), @i = 2
		select @thiskey = index_col(@objname, @indid, 2)
	
		while (@thiskey is not null)
		begin
			select @keys = @keys + ', ' + @thiskey
			select @i = @i + 1
			select @thiskey = index_col(@objname, @indid, @i)
		end

		select @groupname = groupname 
		from sysfilegroups 
		where groupid = @groupid
	
		-- insert row for index
		insert into #indtab values (@indname, @status, @groupname, @keys)

		-- next index
		fetch ind_cursor into @indid, @groupid, @indname, @status
	end

	deallocate ind_cursor

	-- display the results
	select 
		'index_name' = index_name,
		'index_description' = convert(varchar(80),
			case when (stats & 16) <> 0 then 'clustered' else 'nonclustered' end
			+ ' located on ' + groupname),
		'index_keys' = index_keys
	from #indtab
	order by index_name

	return(0)
GO

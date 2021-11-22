-- Reference ATS_CONST_CODES.h for fixed symbol sizes

USE [ATS_DEV]
GO

/****** Object:  StoredProcedure [dbo].[sp_get_position_recs]    Script Date: 12/11/2019 4:09:24 PM ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO




-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE PROCEDURE [dbo].[sp_insert_position_recs]
	@symbol nchar(8),
	@avg_vol int,
	@watch_eligible bit,
	@trade_eligible bit,
	@create_dt DateTime2,
	@update_dt DateTime2,
	@exchange nchar(12),
	@desc nchar (40)
 
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;
	--SET QUOTED_IDENTIFIER ON

	DECLARE @date1 date = @create_dt;
	DECLARE @datetime2_1 datetime2 = @date1;
	SELECT  @datetime2_1 AS '@datetime2_1', @date1 AS '@date1';

	DECLARE @date2 date = @create_dt;
	DECLARE @datetime2_2 datetime2 = @date2;
	SELECT  @datetime2_2 AS '@datetime2_2', @date2 AS '@date2';

	DECLARE @next_seq int = NEXT VALUE FOR [dbo].[WatchPool_Sequence];


    -- Insert statements for procedure here
INSERT INTO [dbo].[watchpool]
     VALUES
           (@next_seq
           ,@symbol
           ,@avg_vol
           ,@watch_eligible
           ,@trade_eligible
           ,@date1
           ,@date2
           ,@exchange
           ,@desc)

	RETURN 1
END




GO

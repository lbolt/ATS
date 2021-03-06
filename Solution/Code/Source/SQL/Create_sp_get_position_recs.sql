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
CREATE PROCEDURE [dbo].[sp_get_position_recs]
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;
	--SET QUOTED_IDENTIFIER ON

	DECLARE @sequence int
	DECLARE @symbol nchar(8)
	DECLARE @avg_vol int
	DECLARE @watch_eligible bit
	DECLARE @trade_eligible bit
	DECLARE @create_dt DateTime2
	DECLARE @update_dt DateTime2
	DECLARE @exchange nchar(12)
	DECLARE @desc nchar (40)



    -- Insert statements for procedure here
	SELECT		*
	FROM		[dbo].[watchpool]
	ORDER		BY sequence

	RETURN 1
END




GO






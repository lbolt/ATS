-- Reference ATS_CONST_CODES.h for fixed symbol sizes


USE [ATS_DEV]
GO

/****** Object:  Sequence [dbo].[WatchPool_Sequence]    Script Date: 12/11/2019 1:54:37 PM ******/

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
CREATE SEQUENCE [dbo].[WatchPool_Sequence] 
 AS [int]
 START WITH 1001
 INCREMENT BY 1
 MINVALUE 1001
 MAXVALUE 2147483647
 CACHE  10 
GO


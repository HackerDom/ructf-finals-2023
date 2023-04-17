FROM mcr.microsoft.com/dotnet/sdk:7.0 AS builder
LABEL stage=builder

COPY ./src /app

WORKDIR /app

RUN dotnet restore && dotnet publish -c Release -o publish

WORKDIR /src

COPY ./src/sneakers/img /app/img

FROM mcr.microsoft.com/dotnet/aspnet:7.0

WORKDIR /app

COPY --from=0 /app ./

CMD ["dotnet", "publish/sneakers.dll"]

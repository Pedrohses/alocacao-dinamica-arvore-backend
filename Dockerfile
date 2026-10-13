FROM gcc:14 AS build
WORKDIR /app
COPY . .
RUN make

FROM debian:bookworm-slim
WORKDIR /app
COPY --from=build /app/servidor /app/arvore.c /app/arvore.h /app/avl.c /app/avl.h ./
ENV HOST=0.0.0.0
ENV PORT=8080
EXPOSE 8080
CMD ["./servidor"]

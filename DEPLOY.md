# Deployment Guide - Railway

## Quick Deploy

### 1. Install Railway CLI

```bash
npm install -g @railway/cli
```

### 2. Login to Railway

```bash
railway login
```

### 3. Create New Project

```bash
cd shopkit_backend
railway init
```

Follow the prompts to:
- Create a new project or link to existing one
- Choose a project name (e.g., `shopkit-production`)

### 4. Add PostgreSQL Database

```bash
railway add postgresql
```

This will automatically:
- Create a PostgreSQL instance
- Set up environment variables (`DATABASE_URL`, `PGHOST`, `PGPORT`, etc.)
- Link the database to your project

### 5. Deploy the Application

```bash
railway up
```

This will:
- Build the Docker image
- Deploy to Railway
- Start the application

### 6. Apply Database Schema

After first deployment, apply the schema:

```bash
railway run psql $DATABASE_URL -f sql/schema.sql
```

Or connect to the database and run the schema manually:

```bash
railway connect postgres
# Then paste the contents of sql/schema.sql
```

### 7. Get Your Application URL

```bash
railway domain
```

Example output:
```
https://shopkit-production.up.railway.app
```

Your API will be available at:
```
https://shopkit-production.up.railway.app/api
```

## Environment Variables

Railway automatically configures these for PostgreSQL:

- `DATABASE_URL` - Full connection string
- `PGHOST` - Database host
- `PGPORT` - Database port (default: 5432)
- `PGDATABASE` - Database name
- `PGUSER` - Database user
- `PGPASSWORD` - Database password

These are automatically injected into your application.

## Configuration Files

### Dockerfile

The Dockerfile is optimized for Railway deployment:
- Uses Ubuntu 22.04 base image
- Installs Drogon framework and dependencies
- Builds the application
- Exposes port 8080

### config.json / config.yaml

Make sure your database configuration in `config.json` or `config.yaml` uses environment variables:

```json
{
  "app": {
    "threads_num": 4,
    "port": 8080,
    "host": "0.0.0.0"
  },
  "db_clients": [
    {
      "name": "default",
      "rdbms": "postgresql",
      "host": "${PGHOST}",
      "port": ${PGPORT},
      "dbname": "${PGDATABASE}",
      "user": "${PGUSER}",
      "passwd": "${PGPASSWORD}",
      "is_fast": true,
      "connection_number": 5
    }
  ]
}
```

## Updating Your Deployment

### Redeploy After Code Changes

```bash
git add .
git commit -m "Update code"
railway up
```

### Update Database Schema

```bash
railway run psql $DATABASE_URL -f sql/schema.sql
```

### View Logs

```bash
railway logs
```

### Restart Service

```bash
railway restart
```

## Troubleshooting

### Build Fails

Check the build logs:
```bash
railway logs --deployment
```

Common issues:
- Missing dependencies in Dockerfile
- CMake configuration errors
- Compilation errors

### Database Connection Issues

Verify database is running:
```bash
railway status
```

Test database connection:
```bash
railway connect postgres
```

### Application Crashes

View runtime logs:
```bash
railway logs
```

Check:
- Database schema is applied
- Environment variables are set correctly
- Port 8080 is exposed and used

## Production Best Practices

### 1. Use Custom Domain

```bash
railway domain
# Then set up a custom domain in Railway dashboard
```

### 2. Enable Health Checks

Add a health check endpoint in your application and configure it in Railway dashboard.

### 3. Monitor Logs

Set up log monitoring:
```bash
railway logs --follow
```

### 4. Backups

Railway provides automatic backups for PostgreSQL. Configure backup retention in the Railway dashboard.

### 5. Scale Resources

Adjust resources in Railway dashboard:
- CPU
- Memory
- Replicas (for high availability)

## Alternative Deployment Methods

### Using Railway Dashboard

1. Go to [railway.app](https://railway.app)
2. Create new project
3. Connect GitHub repository
4. Add PostgreSQL service
5. Deploy automatically on git push

### Manual Docker Deployment

```bash
# Build image
docker build -t shopkit-backend .

# Run locally
docker run -p 8080:8080 \
  -e PGHOST=your-db-host \
  -e PGPORT=5432 \
  -e PGDATABASE=shopkit \
  -e PGUSER=postgres \
  -e PGPASSWORD=yourpassword \
  shopkit-backend
```

## Support

For Railway-specific issues:
- [Railway Documentation](https://docs.railway.app)
- [Railway Discord](https://discord.gg/railway)
- [Railway GitHub](https://github.com/railwayapp/railway)

## Done!

Your ShopKit API is now live at:
```
https://your-app.railway.app/api
```

Test it:
```bash
curl https://your-app.railway.app/api/shops/token/test_shop
```

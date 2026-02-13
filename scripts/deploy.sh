#!/bin/bash

# ShopKit Backend - Production Deployment Script
# Usage: ./scripts/deploy.sh [render|railway|docker]

set -e

DEPLOYMENT_TYPE="${1:-render}"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "🚀 ShopKit Backend Deployment"
echo "=============================="
echo "Deployment type: $DEPLOYMENT_TYPE"
echo "Project directory: $PROJECT_DIR"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

function check_requirements() {
    echo "📋 Checking requirements..."
    
    if ! command -v git &> /dev/null; then
        echo -e "${RED}❌ Git is not installed${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✅ Requirements check passed${NC}\n"
}

function deploy_render() {
    echo "📦 Deploying to Render.com..."
    echo ""
    echo "Manual steps required:"
    echo "1. Go to https://render.com"
    echo "2. Create new Web Service"
    echo "3. Connect your GitHub repository"
    echo "4. Set Build Command: cd shopkit && mkdir -p build && cd build && cmake .. && make -j4"
    echo "5. Set Start Command: cd shopkit/build && ./shopkit"
    echo "6. Add PostgreSQL database"
    echo "7. Set Environment Variable: DATABASE_URL=<internal_database_url>"
    echo "8. Deploy!"
    echo ""
    echo "After deployment, run: ./scripts/init-database.sh <DATABASE_URL>"
}

function deploy_railway() {
    echo "🚂 Deploying to Railway..."
    
    if ! command -v railway &> /dev/null; then
        echo -e "${YELLOW}Installing Railway CLI...${NC}"
        npm install -g @railway/cli
    fi
    
    echo "Logging in to Railway..."
    railway login
    
    echo "Initializing project..."
    railway init
    
    echo "Adding PostgreSQL..."
    railway add postgresql
    
    echo "Deploying..."
    railway up
    
    echo ""
    echo -e "${GREEN}✅ Deployed to Railway!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Run: railway run psql \$DATABASE_URL -f sql/schema.sql"
    echo "2. Get your URL: railway domain"
}

function deploy_docker() {
    echo "🐳 Deploying with Docker..."
    
    if ! command -v docker &> /dev/null; then
        echo -e "${RED}❌ Docker is not installed${NC}"
        exit 1
    fi
    
    if ! command -v docker-compose &> /dev/null; then
        echo -e "${RED}❌ Docker Compose is not installed${NC}"
        exit 1
    fi
    
    # Check if .env exists
    if [ ! -f "$PROJECT_DIR/.env" ]; then
        echo -e "${YELLOW}⚠️  .env file not found, creating from .env.example${NC}"
        cp "$PROJECT_DIR/.env.example" "$PROJECT_DIR/.env"
        echo "Please edit .env file with your configuration"
        exit 1
    fi
    
    echo "Building and starting containers..."
    cd "$PROJECT_DIR"
    docker-compose up -d --build
    
    echo ""
    echo "Waiting for services to be ready..."
    sleep 10
    
    echo ""
    echo -e "${GREEN}✅ Deployed with Docker!${NC}"
    echo "API available at: http://localhost:8080/api"
    echo ""
    echo "Useful commands:"
    echo "  docker-compose logs -f backend  # View logs"
    echo "  docker-compose stop             # Stop services"
    echo "  docker-compose down             # Stop and remove containers"
}

# Main
check_requirements

case "$DEPLOYMENT_TYPE" in
    render)
        deploy_render
        ;;
    railway)
        deploy_railway
        ;;
    docker)
        deploy_docker
        ;;
    *)
        echo -e "${RED}❌ Unknown deployment type: $DEPLOYMENT_TYPE${NC}"
        echo "Usage: $0 [render|railway|docker]"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}🎉 Deployment process complete!${NC}"
echo "Check PROJECT_READY.md for more information"

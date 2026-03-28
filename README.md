# justinottesen.com - My Personal Portfolio Website

A personal portfolio and engineering blog built as a living demonstration of my work. The site itself *is* the portfolio - its architecture, hosting infrastructure, and implementation are all documented within it, and will evolve over time as I build, migrate, and replace components.

I work in distributed systems and application-level logic (consistency models, data propagation across edge appliances), so this site is built to reflect that engineering mindset: functional, pragmatic, data-driven, and incrementally improved rather than flashy or over-engineered.

---

## Philosophy & Goals

- **The website documents itself.** The home page features an interactive architecture diagram showing every component of the site - what it is, why I chose it, how it's configured, and whether I built it or am borrowing it.
- **Incremental migration over big rewrites.** The plan is to start on GitHub Pages, migrate to a cloud VM, then progressively replace external dependencies (web server, etc.) with my own implementations.
- **Data-driven from day one.** Content (projects, resume entries, diagram components) should be stored in structured data files (JSON or YAML), not hardcoded into HTML. The UI renders from the data.
- **No frameworks to start.** Plain HTML, CSS, and vanilla JS. Every line should be explainable. Frameworks can be introduced later if there's a real reason, not out of habit.
- **The repo is part of the portfolio.** Code should be clean, readable, and well-commented. Someone reading the source should be able to follow the reasoning.

---

## Planned Site Structure

### `/` - Home
- Brief personal introduction (a few sentences, not a wall of text)
- Interactive architecture diagram (see below)
- A short callout linking to the Experience page for recruiters

### `/portfolio` - Portfolio
- Cards for projects, each with tags (language, domain, status, etc.)
- Filterable by tag
- Each card links to a detail view or external repo
- Some projects will also appear as nodes in the home page diagram

### `/experience` - Experience
- Resume-style page: work history, skills, education
- Downloadable PDF version (generated from the same structured data source, not maintained separately)
- Targeted at recruiters

### `/about` - About Me
- Long-form text: my journey in programming, how I think about engineering, what I care about
- Will be updated over time
- Written last, once the rest of the site's story is clear

### `/devlog` - DevLog
- Engineering notes: design decisions, tradeoffs, problems I've worked through
- Documents the migration of this site itself as it happens
- Not a lifestyle blog - closer to written RFCs or postmortems

---

## The Architecture Diagram

The centerpiece of the home page. An interactive diagram showing every component of the site. Each node is clickable and shows:
- What the component is
- Whether I built it or am using someone else's
- Configuration notes and reasoning
- A link to the repository if it's something I wrote

### Component Categories (color-coded)

| Category | Description |
|---|---|
| **My own** | Written by me, actively in use |
| **In progress** | Currently using someone else's, actively building my own replacement |
| **External dependency** | Using someone else's, no current plan to replace |
| **Planned** | Not yet implemented, but on the roadmap |
| **Retired** | Built it, replaced it with something better - kept for historical record |

The diagram is generated from a data file (e.g., `architecture.json`), not hardcoded. Adding or updating a component means editing the data file, not the diagram rendering code.

---

## Planned Infrastructure & Migration Path

The site will migrate through stages. Each stage is a chapter in the DevLog and a change in the diagram.

### Stage 1 - GitHub Pages (starting point)
- Static site hosted on GitHub Pages
- Custom domain via CNAME
- Cloudflare in front for DDoS protection and caching
- No build step, no framework

### Stage 2 - Cloud VM
- Migrate off GitHub Pages to a VM (likely Hetzner or DigitalOcean - cheap, simple, real Linux)
- Write and own the deployment pipeline (shell script or lightweight CI)
- Caddy or Nginx as the web server (external dependency, documented in diagram)
- Cloudflare remains in front
- GitHub Pages node in diagram transitions from "external dependency" → "retired"

### Stage 3 - Own HTTP Server
- Replace Caddy/Nginx with an HTTP server I write myself
- Language TBD - likely Go or Rust given performance and systems-level goals
- This is the first major "external dependency" → "my own" transition in the diagram

### Stage 4+ - Further replacement (open-ended)
- TLS handling, deployment tooling, logging/metrics, etc.
- Each piece gets replaced when there's something to learn or demonstrate, not for its own sake

---

## Repository Structure (planned)

```
/
├── index.html
├── portfolio.html
├── experience.html
├── about.html
├── devlog/
│   └── *.html or *.md          # Individual devlog posts
├── assets/
│   ├── css/
│   ├── js/
│   └── fonts/
├── data/
│   ├── architecture.json        # Diagram component definitions
│   ├── projects.json            # Portfolio entries
│   └── resume.json             # Experience page content
├── scripts/
│   └── deploy.sh               # Deployment script (stage 2+)
└── README.md
```

---

## Data Schema (rough intent)

### `architecture.json`
Each node in the diagram:
```json
{
  "id": "web-server",
  "label": "Web Server",
  "category": "external-dependency",
  "description": "Caddy - chosen for automatic HTTPS and simple config. Will be replaced with a custom implementation.",
  "repo": null,
  "links": []
}
```

### `projects.json`
Each portfolio entry:
```json
{
  "id": "http-server",
  "title": "HTTP Server",
  "tags": ["go", "networking", "in-progress"],
  "description": "...",
  "repo": "https://github.com/...",
  "diagram_node": "web-server"
}
```

---

## What This Site Is Not

- Not trying to be visually impressive - the engineering is the point
- Not chasing trends or using tools because they're popular
- Not a static artifact - it will change over time as I build and migrate things

---

## Current Status

> **Stage 0 - Planning.** Nothing is built yet. This README is the starting point.

The first task is getting a minimal home page live on GitHub Pages with the architecture diagram in its initial state (mostly "planned" and "external dependency" nodes). Everything else follows from there.

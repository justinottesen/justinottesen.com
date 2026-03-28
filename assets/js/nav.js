// Marks the nav link matching the current page with aria-current="page".
// Normalises paths by stripping trailing index.html and .html extensions
// so that e.g. /devlog/index.html matches the /devlog link.
document.querySelectorAll('nav a').forEach(a => {
  const normalize = path => path.replace(/\/index\.html$/, '/').replace(/\.html$/, '');
  const linkPath  = normalize(new URL(a.href).pathname);
  const curPath   = normalize(location.pathname);

  const matches = linkPath === '/'
    ? curPath === '/' || curPath === ''
    : curPath === linkPath || curPath.startsWith(linkPath + '/');

  if (matches) a.setAttribute('aria-current', 'page');
});
